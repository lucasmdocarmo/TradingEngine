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
*   **Network Thread (Producer)**: Pinned to Core 2. Runs the `boost::asio` event loop. Reads WebSocket packets, parses JSON (or binary), and pushes `BookTicker` structs to the ring buffer. It never blocks.
*   **Strategy Thread (Consumer)**: Pinned to Core 1. Busy-loops (spins) on the ring buffer. When data arrives, it updates the internal state and executes logic immediately.

### 2. Components

#### `LockFreeQueue<T, Capacity>`
A template-based SPSC queue.
*   **Capacity**: Must be a power of 2 for fast bitwise modulo (`& (Capacity - 1)`).
*   **Cache Safety**: Detailed attention to `alignas(64)` to prevent False Sharing between Head and Tail cache lines.

#### `ObjectPool<T, Size>`
Manages a fixed block of memory.
*   **Acquire**: O(1) retrieval from a LIFO stack (hot cache).
*   **Release**: O(1) return to the stack.
*   **Placement New**: Constructs objects in-place without calling `malloc`.

#### `SymbolManager`
Singleton mapping `std::string` <-> `int`.
*   All internal maps (OrderBook, OrderManager) use `int` keys for maximum hash map performance.

#### `Strategy`
The brain of the system.
*   **Triangular Arbitrage**: Implements O(1) logic to check `USDT -> BTC -> ETH -> USDT` cross-rates on every tick.
*   **books**: Maintains `unordered_map<SymbolId, OrderBook>` to track state of multiple markets simultaneously.

#### `ThreadUtils`
Platform-agnostic wrapper for thread affinity.
*   **Linux**: Uses `pthread_setaffinity_np` for strict core pinning.
*   **macOS**: Uses Mach API `thread_policy_set` for affinity hints (L2 cache localization).
*   **Naming**: Sets thread names for easy debugging in `htop` / `Activity Monitor`.

#### `LatencyMonitor`
Instruments code blocks.
*   Uses `RDTSC` or `std::chrono::high_resolution_clock`.
*   Stores data in stack-allocated buckets (no heap usage).
*   Reports P50, P99, P99.9 latency on shutdown.

## Future Improvements
*   **Kernel Bypass**: Replace `boost::asio` with Solarflare `ef_vi` or DPDK for sub-microsecond networking.
*   **Binary Protocols**: Implement SBE (Simple Binary Encoding) to replace JSON parsing.
*   **Packet Replay**: Add a PCAP replay tool for deterministic backtesting.

## Build & Run
```bash
./setup.sh
```
