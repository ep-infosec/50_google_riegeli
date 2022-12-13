// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef RIEGELI_BASE_RECYCLING_POOL_H_
#define RIEGELI_BASE_RECYCLING_POOL_H_

#include <stddef.h>

#include <atomic>
#include <list>
#include <memory>
#include <utility>
#include <vector>

#include "absl/base/optimization.h"
#include "absl/base/thread_annotations.h"
#include "absl/container/flat_hash_map.h"
#include "absl/synchronization/mutex.h"
#include "riegeli/base/assert.h"
#include "riegeli/base/constexpr.h"
#include "riegeli/base/no_destructor.h"

namespace riegeli {

namespace recycling_pool_internal {
RIEGELI_INLINE_CONSTEXPR(size_t, kDefaultMaxSize, 16);
size_t DefaultGlobalMaxSize();
}  // namespace recycling_pool_internal

// `RecyclingPool<T, Deleter>` keeps a pool of idle objects of type `T`, so that
// instead of creating a new object of type `T`, an existing object can be
// recycled. This is helpful if constructing a new object is more expensive than
// resetting an existing object to the desired state.
//
// Deleter specifies how an object should be eventually deleted, like in
// `std::unique_ptr<T, Deleter>`.
//
// `RecyclingPool` is thread-safe.
template <typename T, typename Deleter = std::default_delete<T>>
class RecyclingPool {
 public:
  // A deleter which puts the object back into the pool.
  class Recycler;

  // A `std::unique_ptr` which puts the object back into the pool instead of
  // deleting it. If a particular object is not suitable for recycling, the
  // `Handle` should have `release()` called and the object can be deleted using
  // the original `Deleter`.
  using Handle = std::unique_ptr<T, Recycler>;

  // A `std::unique_ptr` which deletes the object. If a particular object is
  // suitable for recycling, it can be put back into the pool using `RawPut()`.
  using RawHandle = std::unique_ptr<T, Deleter>;

  // A refurbisher which does nothing; see `Get()`.
  struct DefaultRefurbisher {
    void operator()(T* ptr) const {}
  };

  // The default value of the constructor argument (16).
  static constexpr size_t kDefaultMaxSize =
      recycling_pool_internal::kDefaultMaxSize;

  // The default value of the argument of `global()`.
  //
  // This is the maximum of 16 and the number of available threads.
  static size_t DefaultGlobalMaxSize();

  // Creates a pool with the given maximum number of objects to keep.
  explicit RecyclingPool(size_t max_size = kDefaultMaxSize)
      : max_size_(max_size), ring_buffer_by_freshness_(max_size) {}

  RecyclingPool(const RecyclingPool&) = delete;
  RecyclingPool& operator=(const RecyclingPool&) = delete;

  // Returns a default global pool specific to template parameters of
  // `RecyclingPool`.
  //
  // If called multiple times with different `max_size` arguments, the largest
  // `max_size` is in effect.
  static RecyclingPool& global(size_t max_size = DefaultGlobalMaxSize());

  // Creates an object, or returns an existing object from the pool if possible.
  //
  // `factory` takes no arguments and returns `RawHandle`. It is called to
  // create a new object.
  //
  // If `refurbisher` is specified, it takes a `T*` argument and its result is
  // ignored. It is called before returning an existing object.
  template <typename Factory, typename Refurbisher = DefaultRefurbisher>
  Handle Get(Factory&& factory,
             Refurbisher&& refurbisher = DefaultRefurbisher());

  // Like `Get()`, but the object is not returned into the pool by the
  // destructor of its handle. If the object is suitable for recycling, it can
  // be put back into the pool using `RawPut()`.
  template <typename Factory, typename Refurbisher = DefaultRefurbisher>
  RawHandle RawGet(Factory&& factory,
                   Refurbisher&& refurbisher = DefaultRefurbisher());

  // Puts an idle object into the pool for recycling.
  void RawPut(RawHandle object);

 private:
  void EnsureMaxSize(size_t max_size) ABSL_LOCKS_EXCLUDED(mutex_);

  absl::Mutex mutex_;
  // May be read without holding `mutex_`.
  std::atomic<size_t> max_size_;
  // All objects, ordered by freshness (older to newer).
  size_t ring_buffer_end_ ABSL_GUARDED_BY(mutex_) = 0;
  size_t ring_buffer_size_ ABSL_GUARDED_BY(mutex_) = 0;
  // Invariant:
  //   `ring_buffer_by_freshness_.size() ==
  //        max_size_.load(std::memory_order_relaxed)`
  std::vector<RawHandle> ring_buffer_by_freshness_ ABSL_GUARDED_BY(mutex_);
};

// `KeyedRecyclingPool<T, Key, Deleter>` keeps a pool of idle objects of type
// `T`, so that instead of creating a new object of type `T`, an existing object
// can be recycled. This is helpful if constructing a new object is more
// expensive than resetting an existing object to the desired state.
//
// Deleter specifies how an object should be eventually deleted, like in
// `std::unique_ptr<T, Deleter>`.
//
// The `Key` parameter allows to find an object to reuse only among compatible
// objects, which should be assigned the same key. The `Key` type must be
// equality comparable, hashable (by `absl::Hash`), default constructible, and
// copyable.
//
// `KeyedRecyclingPool` is thread-safe.
template <typename T, typename Key, typename Deleter = std::default_delete<T>>
class KeyedRecyclingPool {
 public:
  // A deleter which puts the object back into the pool.
  class Recycler;

  // A `std::unique_ptr` which puts the object back into the pool instead of
  // deleting it. If a particular object is not suitable for recycling, the
  // `Handle` should have `release()` called and the object can be deleted using
  // the original `Deleter`.
  using Handle = std::unique_ptr<T, Recycler>;

  // A `std::unique_ptr` which deletes the object. If a particular object is
  // suitable for recycling, it can be put back into the pool using `RawPut()`.
  using RawHandle = std::unique_ptr<T, Deleter>;

  // A refurbisher which does nothing; see `Get()`.
  struct DefaultRefurbisher {
    void operator()(T* ptr) const {}
  };

  // The default value of the constructor argument (16).
  static constexpr size_t kDefaultMaxSize =
      recycling_pool_internal::kDefaultMaxSize;

  // The default value of the argument of `global()`.
  //
  // This is the maximum of 16 and the number of available threads.
  static size_t DefaultGlobalMaxSize();

  // Creates a pool with the given maximum number of objects to keep.
  explicit KeyedRecyclingPool(size_t max_size = kDefaultMaxSize)
      : max_size_(max_size), cache_(by_key_.end()) {}

  KeyedRecyclingPool(const KeyedRecyclingPool&) = delete;
  KeyedRecyclingPool& operator=(const KeyedRecyclingPool&) = delete;

  // Returns a default global pool specific to template parameters of
  // `KeyedRecyclingPool`.
  //
  // If called multiple times with different `max_size` arguments, the largest
  // `max_size` is in effect.
  static KeyedRecyclingPool& global(size_t max_size = DefaultGlobalMaxSize());

  // Creates an object, or returns an existing object from the pool if possible.
  //
  // `factory` takes no arguments and returns `RawHandle`. It is called to
  // create a new object.
  //
  // If `refurbisher` is specified, it takes a `T*` argument and its result is
  // ignored. It is called before returning an existing object.
  template <typename Factory, typename Refurbisher = DefaultRefurbisher>
  Handle Get(Key key, Factory&& factory,
             Refurbisher&& refurbisher = DefaultRefurbisher());

  // Like `Get()`, but the object is not returned into the pool by the
  // destructor of its handle. If the object is suitable for recycling, it can
  // be put back into the pool using `RawPut()`.
  template <typename Factory, typename Refurbisher = DefaultRefurbisher>
  RawHandle RawGet(const Key& key, Factory&& factory,
                   Refurbisher&& refurbisher = DefaultRefurbisher());

  // Puts an idle object into the pool for recycling.
  void RawPut(const Key& key, RawHandle object);

 private:
  // Adding or removing elements in `ByFreshness` must not invalidate other
  // iterators.
  using ByFreshness = std::list<Key>;

  struct Entry {
    Entry(RawHandle object, typename ByFreshness::iterator by_freshness_iter)
        : object(std::move(object)), by_freshness_iter(by_freshness_iter) {}

    RawHandle object;
    typename ByFreshness::iterator by_freshness_iter;
  };

  // `std::list` has a smaller overhead than `std::deque` for short sequences.
  using Entries = std::list<Entry>;

  using ByKey = absl::flat_hash_map<Key, Entries>;

  void EnsureMaxSize(size_t max_size);

  std::atomic<size_t> max_size_;
  absl::Mutex mutex_;
  // The key of each object, ordered by the freshness of the object (older to
  // newer).
  ByFreshness by_freshness_ ABSL_GUARDED_BY(mutex_);
  // Objects grouped by their keys. Within each map value the list of objects is
  // non-empty and is ordered by their freshness (older to newer). Each object
  // is associated with the matching `by_freshness_` iterator.
  ByKey by_key_ ABSL_GUARDED_BY(mutex_);
  // Optimization for `Get()` followed by `Put()` with a matching key.
  // If `cache_ != by_key_.end()`, then `cache_->second.back().object` is
  // replaced with `nullptr` instead of erasing its entries.
  typename ByKey::iterator cache_ ABSL_GUARDED_BY(mutex_);
};

// Implementation details follow.

template <typename T, typename Deleter>
class RecyclingPool<T, Deleter>::Recycler : private Deleter {
 public:
  Recycler() {}

  explicit Recycler(RecyclingPool* pool, Deleter&& deleter)
      : Deleter(std::move(deleter)), pool_(pool) {
    RIEGELI_ASSERT(pool_ != nullptr)
        << "Failed precondition of Recycler: null RecyclingPool pointer";
  }

  void operator()(T* ptr) const;

  Deleter& original_deleter() { return *this; }
  const Deleter& original_deleter() const { return *this; }

 private:
  RecyclingPool* pool_ = nullptr;
  // TODO: Use `[[no_unique_address]]` when available instead of
  // relying on empty base optimization.
};

template <typename T, typename Deleter>
inline void RecyclingPool<T, Deleter>::Recycler::operator()(T* ptr) const {
  RIEGELI_ASSERT(pool_ != nullptr)
      << "Failed precondition of RecyclingPool::Recycler: "
         "default-constructed recycler used with an object";
  pool_->RawPut(RawHandle(ptr, original_deleter()));
}

template <typename T, typename Deleter>
inline size_t RecyclingPool<T, Deleter>::DefaultGlobalMaxSize() {
  return recycling_pool_internal::DefaultGlobalMaxSize();
}

template <typename T, typename Deleter>
RecyclingPool<T, Deleter>& RecyclingPool<T, Deleter>::global(size_t max_size) {
  static NoDestructor<RecyclingPool> kStaticRecyclingPool(max_size);
  kStaticRecyclingPool->EnsureMaxSize(max_size);
  return *kStaticRecyclingPool;
}

template <typename T, typename Deleter>
inline void RecyclingPool<T, Deleter>::EnsureMaxSize(size_t max_size) {
  if (ABSL_PREDICT_FALSE(max_size_.load(std::memory_order_relaxed) >=
                         max_size)) {
    return;
  }
  absl::MutexLock lock(&mutex_);
  if (ABSL_PREDICT_FALSE(max_size_.load(std::memory_order_relaxed) >=
                         max_size)) {
    return;
  }
  const size_t old_size =
      max_size_.exchange(max_size, std::memory_order_relaxed);
  std::vector<RawHandle> new_ring_buffer(max_size);
  size_t old_idx = ring_buffer_end_;
  ring_buffer_end_ = ring_buffer_size_;
  size_t new_idx = ring_buffer_end_;
  while (new_idx > 0) {
    old_idx = old_idx == 0 ? old_size - 1 : old_idx - 1;
    --new_idx;
    new_ring_buffer[new_idx] = std::move(ring_buffer_by_freshness_[old_idx]);
  }
  ring_buffer_by_freshness_ = std::move(new_ring_buffer);
}

template <typename T, typename Deleter>
template <typename Factory, typename Refurbisher>
typename RecyclingPool<T, Deleter>::Handle RecyclingPool<T, Deleter>::Get(
    Factory&& factory, Refurbisher&& refurbisher) {
  RawHandle returned = RawGet(std::forward<Factory>(factory),
                              std::forward<Refurbisher>(refurbisher));
  return Handle(returned.release(),
                Recycler(this, std::move(returned.get_deleter())));
}

template <typename T, typename Deleter>
template <typename Factory, typename Refurbisher>
typename RecyclingPool<T, Deleter>::RawHandle RecyclingPool<T, Deleter>::RawGet(
    Factory&& factory, Refurbisher&& refurbisher) {
  RawHandle returned;
  {
    absl::MutexLock lock(&mutex_);
    if (ABSL_PREDICT_TRUE(ring_buffer_size_ > 0)) {
      ring_buffer_end_ = ring_buffer_end_ == 0
                             ? max_size_.load(std::memory_order_relaxed) - 1
                             : ring_buffer_end_ - 1;
      // Return the newest entry.
      returned = std::move(ring_buffer_by_freshness_[ring_buffer_end_]);
      --ring_buffer_size_;
    }
  }
  if (ABSL_PREDICT_TRUE(returned != nullptr)) {
    std::forward<Refurbisher>(refurbisher)(returned.get());
  } else {
    returned = std::forward<Factory>(factory)();
  }
  return returned;
}

template <typename T, typename Deleter>
void RecyclingPool<T, Deleter>::RawPut(RawHandle object) {
  RawHandle evicted;
  absl::MutexLock lock(&mutex_);
  // Add a newest entry. Evict the oldest entry if the pool is full.
  if (ABSL_PREDICT_FALSE(ring_buffer_by_freshness_.empty())) return;
  evicted = std::exchange(ring_buffer_by_freshness_[ring_buffer_end_],
                          std::move(object));
  ring_buffer_end_ =
      ring_buffer_end_ + 1 == max_size_.load(std::memory_order_relaxed)
          ? 0
          : ring_buffer_end_ + 1;
  if (ABSL_PREDICT_TRUE(ring_buffer_size_ <
                        max_size_.load(std::memory_order_relaxed))) {
    ++ring_buffer_size_;
  }
  // Destroy `evicted` after releasing `mutex_`.
}

template <typename T, typename Key, typename Deleter>
class KeyedRecyclingPool<T, Key, Deleter>::Recycler : private Deleter {
 public:
  Recycler() {}

  explicit Recycler(KeyedRecyclingPool* pool, Key&& key, Deleter&& deleter)
      : Deleter(std::move(deleter)), pool_(pool), key_(std::move(key)) {
    RIEGELI_ASSERT(pool_ != nullptr)
        << "Failed precondition of Recycler: null KeyedRecyclingPool pointer";
  }

  void operator()(T* ptr) const;

  Deleter& original_deleter() { return *this; }
  const Deleter& original_deleter() const { return *this; }

 private:
  KeyedRecyclingPool* pool_ = nullptr;
  Key key_;
  // TODO: Use `[[no_unique_address]]` when available instead of
  // relying on empty base optimization.
};

template <typename T, typename Key, typename Deleter>
inline void KeyedRecyclingPool<T, Key, Deleter>::Recycler::operator()(
    T* ptr) const {
  RIEGELI_ASSERT(pool_ != nullptr)
      << "Failed precondition of KeyedRecyclingPool::Recycler: "
         "default-constructed recycler used with an object";
  pool_->RawPut(key_, RawHandle(ptr, original_deleter()));
}

template <typename T, typename Key, typename Deleter>
inline size_t KeyedRecyclingPool<T, Key, Deleter>::DefaultGlobalMaxSize() {
  return recycling_pool_internal::DefaultGlobalMaxSize();
}

template <typename T, typename Key, typename Deleter>
KeyedRecyclingPool<T, Key, Deleter>&
KeyedRecyclingPool<T, Key, Deleter>::global(size_t max_size) {
  static NoDestructor<KeyedRecyclingPool> kStaticKeyedRecyclingPool(max_size);
  kStaticKeyedRecyclingPool->EnsureMaxSize(max_size);
  return *kStaticKeyedRecyclingPool;
}

template <typename T, typename Key, typename Deleter>
inline void KeyedRecyclingPool<T, Key, Deleter>::EnsureMaxSize(
    size_t max_size) {
  size_t previous_size = max_size_.load(std::memory_order_relaxed);
  while (ABSL_PREDICT_FALSE(previous_size < max_size)) {
    if (max_size_.compare_exchange_weak(previous_size, max_size,
                                        std::memory_order_relaxed))
      break;
  }
}

template <typename T, typename Key, typename Deleter>
template <typename Factory, typename Refurbisher>
typename KeyedRecyclingPool<T, Key, Deleter>::Handle
KeyedRecyclingPool<T, Key, Deleter>::Get(Key key, Factory&& factory,
                                         Refurbisher&& refurbisher) {
  RawHandle returned = RawGet(key, std::forward<Factory>(factory),
                              std::forward<Refurbisher>(refurbisher));
  return Handle(
      returned.release(),
      Recycler(this, std::move(key), std::move(returned.get_deleter())));
}

template <typename T, typename Key, typename Deleter>
template <typename Factory, typename Refurbisher>
typename KeyedRecyclingPool<T, Key, Deleter>::RawHandle
KeyedRecyclingPool<T, Key, Deleter>::RawGet(const Key& key, Factory&& factory,
                                            Refurbisher&& refurbisher) {
  RawHandle returned;
  {
    absl::MutexLock lock(&mutex_);
    if (cache_ != by_key_.end()) {
      // Finish erasing the cached entry.
      Entries& entries = cache_->second;
      RIEGELI_ASSERT(!entries.empty())
          << "Failed invariant of KeyedRecyclingPool: "
             "empty by_key_ value";
      RIEGELI_ASSERT(entries.back().object == nullptr)
          << "Failed invariant of KeyedRecyclingPool: "
             "non-nullptr object pointed to by cache_";
      by_freshness_.erase(entries.back().by_freshness_iter);
      entries.pop_back();
      if (entries.empty()) by_key_.erase(cache_);
    }
    const typename ByKey::iterator by_key_iter = by_key_.find(key);
    if (ABSL_PREDICT_TRUE(by_key_iter != by_key_.end())) {
      // Return the newest entry with this key.
      Entries& entries = by_key_iter->second;
      RIEGELI_ASSERT(!entries.empty())
          << "Failed invariant of KeyedRecyclingPool: "
             "empty by_key_ value";
      RIEGELI_ASSERT(entries.back().object != nullptr)
          << "Failed invariant of KeyedRecyclingPool: "
             "nullptr object not pointed to by cache_";
      returned = std::move(entries.back().object);
    }
    cache_ = by_key_iter;
  }
  if (ABSL_PREDICT_TRUE(returned != nullptr)) {
    std::forward<Refurbisher>(refurbisher)(returned.get());
  } else {
    returned = std::forward<Factory>(factory)();
  }
  return returned;
}

template <typename T, typename Key, typename Deleter>
void KeyedRecyclingPool<T, Key, Deleter>::RawPut(const Key& key,
                                                 RawHandle object) {
  RawHandle evicted;
  absl::MutexLock lock(&mutex_);
  // Add a newest entry with this key.
  if (ABSL_PREDICT_TRUE(cache_ != by_key_.end())) {
    Entries& entries = cache_->second;
    RIEGELI_ASSERT(!entries.empty())
        << "Failed invariant of KeyedRecyclingPool: "
           "empty by_key_ value";
    if (ABSL_PREDICT_TRUE(cache_->first == key)) {
      // `cache_` hit. Set the object pointer again.
      RIEGELI_ASSERT(entries.back().object == nullptr)
          << "Failed invariant of KeyedRecyclingPool: "
             "non-nullptr object pointed to by cache_";
      entries.back().object = std::move(object);
      cache_ = by_key_.end();
      return;
    }
    // `cache_` miss. Finish erasing the cached entry.
    by_freshness_.erase(entries.back().by_freshness_iter);
    entries.pop_back();
    if (entries.empty()) by_key_.erase(cache_);
  }
  by_freshness_.push_back(key);
  typename ByFreshness::iterator by_freshness_iter = by_freshness_.end();
  --by_freshness_iter;
  // This invalidates `by_key_` iterators, including `cache_`.
  by_key_[key].emplace_back(std::move(object), by_freshness_iter);
  if (ABSL_PREDICT_FALSE(by_freshness_.size() >
                         max_size_.load(std::memory_order_relaxed))) {
    // Evict the oldest entry.
    const Key& evicted_key = by_freshness_.front();
    const typename ByKey::iterator by_key_iter = by_key_.find(evicted_key);
    RIEGELI_ASSERT(by_key_iter != by_key_.end())
        << "Failed invariant of KeyedRecyclingPool: "
           "a key from by_freshness_ absent in by_key_";
    Entries& entries = by_key_iter->second;
    RIEGELI_ASSERT(!entries.empty())
        << "Failed invariant of KeyedRecyclingPool: "
           "empty by_key_ value";
    RIEGELI_ASSERT(entries.back().object != nullptr)
        << "Failed invariant of KeyedRecyclingPool: "
           "nullptr object not pointed to by cache_";
    evicted = std::move(entries.front().object);
    entries.pop_front();
    if (entries.empty()) by_key_.erase(by_key_iter);
    by_freshness_.pop_front();
  }
  cache_ = by_key_.end();
  // Destroy `evicted` after releasing `mutex_`.
}

}  // namespace riegeli

#endif  // RIEGELI_BASE_RECYCLING_POOL_H_
