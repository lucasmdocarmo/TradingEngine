# High-Frequency Trading System Architecture

## Overview
This project demonstrates a multi-threaded, low-latency execution system for algorithmic trading. It is designed to handle high-throughput market data and execute strategy logic with sub-microsecond latency.

## Key Performance Features
1.  **Lock-Free Inter-Thread Communication**: Uses a Single-Producer Single-Consumer (SPSC) ring buffer with C++ atomics (`memory_order_acquire`/`release`) to pass data between the Network and Strategy threads without mutexes.
2.  **Zero-Allocation Runtime**:
    *   **Object Pools**: Orders and event objects are pre-allocated at startup.
    *   **Symbol Interpolation**: String symbols ("BTCUSDT") are converted to integer IDs (`SymbolId`) at startup. The hot path uses only integer comparisons (O(1)).
3.  **Latency Monitoring**: A zero-overhead `LatencyMonitor` instruments the critical path, recording nanosecond-level histograms to prove performance characteristics.
4.  **Thread Pinning & Isolation**:
    *   **Strategic Core Affinity**: The Strategy thread is pinned to a dedicated CPU core (or affinity tag on macOS) to prevent OS context switching and preserve L1/L2 cache locality.
    *   **Network Separation**: The Network thread is pinned to a separate core to handle interrupt load without impacting strategy decision latency.

## Architecture

### 1. Threads
*   **Network Thread (Producer)**: Pinned to Core 2. Runs the `boost::asio` event loop. Reads WebSocket packets, parses JSON, and pushes `BookTicker` structs to the ring buffer. It never blocks.
*   **Strategy Thread (Consumer)**: Pinned to Core 1. Busy-loops (spins) on the ring buffer. When data arrives, it updates the internal state and executes logic immediately.

### 2. Core Components

#### `LockFreeQueue<T, Capacity>`
A template-based SPSC ring buffer using C++20 atomics.
*   **Zero-Copy**: Passes data by value (small structs) or pointer to pre-allocated buffers.
*   **Cache Friendliness**: Head and Tail pointers are padded to 64 bytes (`alignas(64)`) to prevent False Sharing.

#### `ObjectPool<T, Size>`
Manages a fixed block of memory for critical objects (Orders).
*   **O(1) Allocation**: Retrieves from a LIFO stack (hot cache) in nanoseconds.
*   **No Fragment**: Eliminates heap fragmentation and `malloc` syscall overhead.

#### `RiskManager` (Pre-Trade Filters)
The guardian of the system. Implements 4 layers of checks before any order leaves the strategy:
1.  **Fat Finger**: Rejects orders > MaxSize.
2.  **Position Limit**: Checks if projected position exceeds MaxPosition.
3.  **Price Collar**: Rejects orders > 5% away from market mid-price.
4.  **Throttle**: Limits order rate to 10 orders/sec (Token Bucket).

#### `OrderManager` (OMS) & `OrderGateway` (EMS)
*   **OMS**: Tracks the lifecycle of every order (`New` -> `Pending` -> `Filled`). Handles asynchronous `ExecutionReport` updates.
*   **EMS**: Routes orders to the exchange. Simulates network latency (5-50ms) and matching engine mechanics for realistic testing.
*   **Async Feedback**: The OMS updates strategy state only when the "Exchange" confirms the fill, preventing race conditions.

#### `Strategy` & Signals
The decision engine.
*   **Triangular Arbitrage**: Checks `USDT -> BTC -> ETH -> USDT` cross-rates for risk-free profit.
*   **Alpha Signal**: Monitors **Order Book Imbalance** on BTCUSDT. Calculation: `(BidQty - AskQty) / (BidQty + AskQty)`. Triggers aggressive orders when imbalance > 0.8.

#### `MarketDataReplay`
Deterministic Backtesting Engine.
*   Reads historical tick data from `market_data.csv`.
*   Feeds the strategy at maximum speed to validate logic and signals offline.

## Performance Metrics
*   **Tick-to-Trade Latency**: ~900 nanoseconds (internal logic).
*   **Risk Check Overhead**: < 200 nanoseconds per order.
*   **Jitter**: P99 latency < 3 microseconds (on macOS).

## Build & Run
```bash
# Build
./setup.sh

# Run Live
./build/execution_engine

# Run Replay (Backtest)
./build/execution_engine --replay market_data.csv

# Analyze Results
python3 analyze_performance.py execution.log
```
