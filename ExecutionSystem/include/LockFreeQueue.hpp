#pragma once
#include <atomic>
#include <cstddef>
#include <iostream>
#include <vector>

namespace quant {

// A Lock-Free Single-Producer Single-Consumer (SPSC) Ring Buffer.
// This data structure is critical for HFT systems because it allows
// passing messages between threads (e.g., Market Data -> Strategy)
// without using mutexes, which introduce expensive context switches.
template <typename T, size_t Capacity> class LockFreeQueue {
public:
  // Ensure capacity is a power of 2 for bitwise modulo optimization
  static_assert((Capacity & (Capacity - 1)) == 0,
                "Capacity must be a power of 2");

  LockFreeQueue() : head_(0), tail_(0) {}

  // Push an item into the queue. Returns true if successful, false if full.
  // This is called by the PRODUCER thread (Market Data Thread).
  bool push(const T &item) {
    // Load the current tail index. We use memory_order_relaxed because
    // only the producer writes to tail_, so we don't need synchronization here.
    size_t current_tail = tail_.load(std::memory_order_relaxed);

    // Calculate the next tail position.
    // Using bitwise AND is faster than modulo (%) for power-of-2 capacity.
    // Example: If Capacity is 1024 (10000000000 in binary), Mask is 1023
    // (01111111111).
    size_t next_tail = (current_tail + 1) & (Capacity - 1);

    // Check if the queue is full.
    // We need to load head_ to see where the consumer is.
    // memory_order_acquire: Ensures that we see the latest value of head_
    // written by the consumer thread.
    if (next_tail == head_.load(std::memory_order_acquire)) {
      return false; // Queue is full
    }

    // Store the item in the buffer.
    buffer_[current_tail] = item;

    // Update the tail index to make the new item visible to the consumer.
    // memory_order_release: Ensures that the write to buffer_[current_tail]
    // happens BEFORE the update to tail_. The consumer, reading tail_ with
    // acquire, will then be guaranteed to see the data.
    tail_.store(next_tail, std::memory_order_release);
    return true;
  }

  // Pop an item from the queue. Returns true if successful, false if empty.
  // This is called by the CONSUMER thread (Strategy Thread).
  bool pop(T &item) {
    // Load the current head index. Only consumer writes to head_, so relaxed is
    // fine.
    size_t current_head = head_.load(std::memory_order_relaxed);

    // Check if the queue is empty.
    // We need to load tail_ to see where the producer has written up to.
    // memory_order_acquire: Ensures that all writes to buffer_ done by the
    // producer (before its release-store to tail_) are visible to us here.
    if (current_head == tail_.load(std::memory_order_acquire)) {
      return false; // Queue is empty
    }

    // Read the item from the buffer.
    item = buffer_[current_head];

    // Calculate the next head position.
    size_t next_head = (current_head + 1) & (Capacity - 1);

    // Update the head index to free up the slot.
    // memory_order_release: Ensures that our read of the item is finished
    // before we tell the producer that this slot is free to be overwritten.
    head_.store(next_head, std::memory_order_release);
    return true;
  }

  // Check if the queue is empty
  bool empty() const {
    return head_.load(std::memory_order_acquire) ==
           tail_.load(std::memory_order_acquire);
  }

private:
  std::vector<T> buffer_{Capacity}; // The actual storage

  // alignas(64) ensures that head_ and tail_ are on different cache lines.
  // This prevents "False Sharing", where one core invalidates the cache line
  // of another core just because they are accessing adjacent variables.
  // 64 bytes is the standard cache line size for x86_64 CPUs.
  alignas(64) std::atomic<size_t> head_; // Index of the next item to read
  alignas(64)
      std::atomic<size_t> tail_; // Index where the next item will be written
};

} // namespace quant
