#pragma once
#include <cassert>
#include <iostream>
#include <vector>

namespace quant {

// A high-performance Object Pool for zero-allocation runtime.
// Pre-allocates a large chunk of memory at startup and manages object lifecycle
// via a free-list stack.
// Allocation/Deallocation is O(1).
template <typename T, size_t PoolSize> class ObjectPool {
public:
  ObjectPool() {
    // Reserve memory for all objects
    pool_.resize(PoolSize);

    // Initialize the free list with pointers to all objects
    // LIFO (Last-In, First-Out) stack ensures cache locality (hot objects
    // reused).
    freeList_.reserve(PoolSize);
    for (size_t i = 0; i < PoolSize; ++i) {
      freeList_.push_back(&pool_[i]);
    }
  }

  // Acquire an object from the pool. Returns nullptr if empty.
  // Construct arguments are forwarded to the object's constructor (placement
  // new).
  template <typename... Args> T *acquire(Args &&...args) {
    if (freeList_.empty()) {
      std::cerr << "ObjectPool Exhausted! Size: " << PoolSize << std::endl;
      return nullptr;
    }

    T *obj = freeList_.back();
    freeList_.pop_back();

    // Use placement new to construct the object in pre-allocated memory
    new (obj) T(std::forward<Args>(args)...);
    return obj;
  }

  // Release an object back to the pool.
  void release(T *obj) {
    // Create space for new object by destructing the old one
    obj->~T();

    // Push back to free list
    freeList_.push_back(obj);
  }

private:
  std::vector<T> pool_;       // Contiguous memory block
  std::vector<T *> freeList_; // Stack of available pointers
};

} // namespace quant
