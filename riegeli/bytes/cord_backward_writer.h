// Copyright 2017 Google LLC
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

#ifndef RIEGELI_BYTES_CORD_BACKWARD_WRITER_H_
#define RIEGELI_BYTES_CORD_BACKWARD_WRITER_H_

#include <stddef.h>
#include <stdint.h>

#include <tuple>
#include <type_traits>
#include <utility>

#include "absl/base/attributes.h"
#include "absl/strings/cord.h"
#include "absl/strings/cord_buffer.h"
#include "absl/types/optional.h"
#include "riegeli/base/arithmetic.h"
#include "riegeli/base/assert.h"
#include "riegeli/base/buffer.h"
#include "riegeli/base/buffering.h"
#include "riegeli/base/chain.h"
#include "riegeli/base/dependency.h"
#include "riegeli/base/object.h"
#include "riegeli/base/types.h"
#include "riegeli/bytes/backward_writer.h"

namespace riegeli {

// Template parameter independent part of `CordBackwardWriter`.
class CordBackwardWriterBase : public BackwardWriter {
 public:
  class Options {
   public:
    Options() noexcept {}

    // If `false`, replaces existing contents of the destination, clearing it
    // first.
    //
    // If `true`, prepends to existing contents of the destination.
    //
    // Default: `false`.
    Options& set_prepend(bool prepend) & {
      prepend_ = prepend;
      return *this;
    }
    Options&& set_prepend(bool prepend) && {
      return std::move(set_prepend(prepend));
    }
    bool prepend() const { return prepend_; }

    // Minimal size of a block of allocated data.
    //
    // This is used initially, while the destination is small.
    //
    // Default: `kDefaultMinBlockSize` (256).
    Options& set_min_block_size(size_t min_block_size) & {
      min_block_size_ = UnsignedMin(min_block_size, uint32_t{1} << 31);
      return *this;
    }
    Options&& set_min_block_size(size_t min_block_size) && {
      return std::move(set_min_block_size(min_block_size));
    }
    size_t min_block_size() const { return min_block_size_; }

    // Maximal size of a block of allocated data.
    //
    // This is for performance tuning, not a guarantee: does not apply to
    // objects allocated separately and then written to this
    // `CordBackwardWriter`.
    //
    // Default: `kDefaultMaxBlockSize - 13` (65523).
    Options& set_max_block_size(size_t max_block_size) & {
      RIEGELI_ASSERT_GT(max_block_size, 0u)
          << "Failed precondition of "
             "CordBackwardWriterBase::Options::set_max_block_size(): "
             "zero block size";
      max_block_size_ = UnsignedMin(max_block_size, uint32_t{1} << 31);
      return *this;
    }
    Options&& set_max_block_size(size_t max_block_size) && {
      return std::move(set_max_block_size(max_block_size));
    }
    size_t max_block_size() const { return max_block_size_; }

   private:
    bool prepend_ = false;
    // Use `uint32_t` instead of `size_t` to reduce the object size.
    uint32_t min_block_size_ = uint32_t{kDefaultMinBlockSize};
    uint32_t max_block_size_ =
        uint32_t{absl::CordBuffer::MaximumPayload(kDefaultMaxBlockSize)};
  };

  // Returns the `absl::Cord` being written to. Unchanged by `Close()`.
  virtual absl::Cord* DestCord() = 0;
  virtual const absl::Cord* DestCord() const = 0;

  bool SupportsTruncate() override { return true; }

 protected:
  explicit CordBackwardWriterBase(Closed) noexcept : BackwardWriter(kClosed) {}

  explicit CordBackwardWriterBase(const Options& options);

  CordBackwardWriterBase(CordBackwardWriterBase&& that) noexcept;
  CordBackwardWriterBase& operator=(CordBackwardWriterBase&& that) noexcept;

  void Reset(Closed);
  void Reset(const Options& options);
  void Initialize(absl::Cord* dest, bool prepend);

  void Done() override;
  void SetWriteSizeHintImpl(absl::optional<Position> write_size_hint) override;
  bool PushSlow(size_t min_length, size_t recommended_length) override;
  using BackwardWriter::WriteSlow;
  bool WriteSlow(const Chain& src) override;
  bool WriteSlow(Chain&& src) override;
  bool WriteSlow(const absl::Cord& src) override;
  bool WriteSlow(absl::Cord&& src) override;
  bool WriteZerosSlow(Position length) override;
  bool FlushImpl(FlushType flush_type) override;
  bool TruncateImpl(Position new_size) override;

 private:
  static constexpr size_t kCordBufferBlockSize =
      UnsignedMin(kDefaultMaxBlockSize, absl::CordBuffer::kCustomLimit);
  static constexpr size_t kCordBufferMaxSize =
      absl::CordBuffer::MaximumPayload(kCordBufferBlockSize);

  // If the buffer is not empty, prepends it to `dest`.
  void SyncBuffer(absl::Cord& dest);

  absl::optional<Position> size_hint_;
  // Use `uint32_t` instead of `size_t` to reduce the object size.
  uint32_t min_block_size_ = uint32_t{kDefaultMinBlockSize};
  uint32_t max_block_size_ =
      uint32_t{absl::CordBuffer::MaximumPayload(kDefaultMaxBlockSize)};

  // Buffered data to be prepended, in either `cord_buffer_` or `buffer_`.
  absl::CordBuffer cord_buffer_;
  Buffer buffer_;

  // Invariants:
  //   `limit() == nullptr` or `limit() == cord_buffer_.data()`
  //       or `limit() == buffer_.data()`
  //   if `ok()` then `start_pos() == DestCord()->size()`
};

// A `Writer` which prepends to an `absl::Cord`.
//
// The `Dest` template parameter specifies the type of the object providing and
// possibly owning the `absl::Cord` being written to. `Dest` must support
// `Dependency<absl::Cord*, Dest>`, e.g. `absl::Cord*` (not owned, default),
// `absl::Cord` (owned).
//
// By relying on CTAD the template argument can be deduced as `absl::Cord`
// if there are no constructor arguments or the only argument is `Options`,
// otherwise as the value type of the first constructor argument, except that
// CTAD is deleted if the first constructor argument is an `absl::Cord&` or
// `const absl::Cord&` (to avoid writing to an unintentionally separate copy of
// an existing object). This requires C++17.
//
// The `absl::Cord` must not be accessed until the `CordBackwardWriter` is
// closed or no longer used.
template <typename Dest = absl::Cord*>
class CordBackwardWriter : public CordBackwardWriterBase {
 public:
  // Creates a closed `CordBackwardWriter`.
  explicit CordBackwardWriter(Closed) noexcept
      : CordBackwardWriterBase(kClosed) {}

  // Will prepend to the `absl::Cord` provided by `dest`.
  explicit CordBackwardWriter(const Dest& dest, Options options = Options());
  explicit CordBackwardWriter(Dest&& dest, Options options = Options());

  // Will prepend to the `absl::Cord` provided by a `Dest` constructed from
  // elements of `dest_args`. This avoids constructing a temporary `Dest` and
  // moving from it.
  template <typename... DestArgs>
  explicit CordBackwardWriter(std::tuple<DestArgs...> dest_args,
                              Options options = Options());

  // Will append to an owned `absl::Cord` which can be accessed by `dest()`.
  // This constructor is present only if `Dest` is `absl::Cord`.
  template <
      typename DependentDest = Dest,
      std::enable_if_t<std::is_same<DependentDest, absl::Cord>::value, int> = 0>
  explicit CordBackwardWriter(Options options = Options());

  CordBackwardWriter(CordBackwardWriter&& that) noexcept;
  CordBackwardWriter& operator=(CordBackwardWriter&& that) noexcept;

  // Makes `*this` equivalent to a newly constructed `CordBackwardWriter`. This
  // avoids constructing a temporary `CordBackwardWriter` and moving from it.
  ABSL_ATTRIBUTE_REINITIALIZES void Reset(Closed);
  ABSL_ATTRIBUTE_REINITIALIZES void Reset(const Dest& dest,
                                          Options options = Options());
  ABSL_ATTRIBUTE_REINITIALIZES void Reset(Dest&& dest,
                                          Options options = Options());
  template <typename... DestArgs>
  ABSL_ATTRIBUTE_REINITIALIZES void Reset(std::tuple<DestArgs...> dest_args,
                                          Options options = Options());
  template <
      typename DependentDest = Dest,
      std::enable_if_t<std::is_same<DependentDest, absl::Cord>::value, int> = 0>
  ABSL_ATTRIBUTE_REINITIALIZES void Reset(Options options = Options());

  // Returns the object providing and possibly owning the `absl::Cord` being
  // written to. Unchanged by `Close()`.
  Dest& dest() { return dest_.manager(); }
  const Dest& dest() const { return dest_.manager(); }
  absl::Cord* DestCord() override { return dest_.get(); }
  const absl::Cord* DestCord() const override { return dest_.get(); }

 private:
  // The object providing and possibly owning the `absl::Cord` being written to.
  Dependency<absl::Cord*, Dest> dest_;
};

// Support CTAD.
#if __cpp_deduction_guides
explicit CordBackwardWriter(Closed)->CordBackwardWriter<DeleteCtad<Closed>>;
template <typename Dest>
explicit CordBackwardWriter(
    const Dest& dest,
    CordBackwardWriterBase::Options options = CordBackwardWriterBase::Options())
    -> CordBackwardWriter<std::conditional_t<
        std::is_convertible<const Dest*, const absl::Cord*>::value,
        DeleteCtad<const Dest&>, std::decay_t<Dest>>>;
template <typename Dest>
explicit CordBackwardWriter(
    Dest&& dest,
    CordBackwardWriterBase::Options options = CordBackwardWriterBase::Options())
    -> CordBackwardWriter<std::conditional_t<
        std::is_lvalue_reference<Dest>::value &&
            std::is_convertible<std::remove_reference_t<Dest>*,
                                const absl::Cord*>::value,
        DeleteCtad<Dest&&>, std::decay_t<Dest>>>;
template <typename... DestArgs>
explicit CordBackwardWriter(
    std::tuple<DestArgs...> dest_args,
    CordBackwardWriterBase::Options options = CordBackwardWriterBase::Options())
    -> CordBackwardWriter<DeleteCtad<std::tuple<DestArgs...>>>;
explicit CordBackwardWriter(
    CordBackwardWriterBase::Options options = CordBackwardWriterBase::Options())
    ->CordBackwardWriter<absl::Cord>;
#endif

// Implementation details follow.

inline CordBackwardWriterBase::CordBackwardWriterBase(const Options& options)
    : min_block_size_(IntCast<uint32_t>(options.min_block_size())),
      max_block_size_(IntCast<uint32_t>(options.max_block_size())) {}

inline CordBackwardWriterBase::CordBackwardWriterBase(
    CordBackwardWriterBase&& that) noexcept
    : BackwardWriter(static_cast<BackwardWriter&&>(that)),
      size_hint_(that.size_hint_),
      min_block_size_(that.min_block_size_),
      max_block_size_(that.max_block_size_),
      buffer_(std::move(that.buffer_)) {
  if (limit() == that.cord_buffer_.data()) {
    cord_buffer_ = std::move(that.cord_buffer_);
    set_buffer(cord_buffer_.data(), start_to_limit(), start_to_cursor());
  } else {
    cord_buffer_ = std::move(that.cord_buffer_);
  }
}

inline CordBackwardWriterBase& CordBackwardWriterBase::operator=(
    CordBackwardWriterBase&& that) noexcept {
  BackwardWriter::operator=(static_cast<BackwardWriter&&>(that));
  size_hint_ = that.size_hint_;
  min_block_size_ = that.min_block_size_;
  max_block_size_ = that.max_block_size_;
  buffer_ = std::move(that.buffer_);
  if (limit() == that.cord_buffer_.data()) {
    cord_buffer_ = std::move(that.cord_buffer_);
    set_buffer(cord_buffer_.data(), start_to_limit(), start_to_cursor());
  } else {
    cord_buffer_ = std::move(that.cord_buffer_);
  }
  return *this;
}

inline void CordBackwardWriterBase::Reset(Closed) {
  BackwardWriter::Reset(kClosed);
  size_hint_ = absl::nullopt;
  min_block_size_ = uint32_t{kDefaultMinBlockSize};
  max_block_size_ =
      uint32_t{absl::CordBuffer::MaximumPayload(kDefaultMaxBlockSize)};
  cord_buffer_ = absl::CordBuffer();
  buffer_ = Buffer();
}

inline void CordBackwardWriterBase::Reset(const Options& options) {
  BackwardWriter::Reset();
  size_hint_ = absl::nullopt;
  min_block_size_ = IntCast<uint32_t>(options.min_block_size());
  max_block_size_ = IntCast<uint32_t>(options.max_block_size());
}

inline void CordBackwardWriterBase::Initialize(absl::Cord* dest, bool prepend) {
  RIEGELI_ASSERT(dest != nullptr)
      << "Failed precondition of CordBackwardWriter: null Cord pointer";
  if (prepend) {
    set_start_pos(dest->size());
  } else {
    dest->Clear();
  }
}

template <typename Dest>
inline CordBackwardWriter<Dest>::CordBackwardWriter(const Dest& dest,
                                                    Options options)
    : CordBackwardWriterBase(options), dest_(dest) {
  Initialize(dest_.get(), options.prepend());
}

template <typename Dest>
inline CordBackwardWriter<Dest>::CordBackwardWriter(Dest&& dest,
                                                    Options options)
    : CordBackwardWriterBase(options), dest_(std::move(dest)) {
  Initialize(dest_.get(), options.prepend());
}

template <typename Dest>
template <typename... DestArgs>
inline CordBackwardWriter<Dest>::CordBackwardWriter(
    std::tuple<DestArgs...> dest_args, Options options)
    : CordBackwardWriterBase(options), dest_(std::move(dest_args)) {
  Initialize(dest_.get(), options.prepend());
}

template <typename Dest>
template <typename DependentDest,
          std::enable_if_t<std::is_same<DependentDest, absl::Cord>::value, int>>
inline CordBackwardWriter<Dest>::CordBackwardWriter(Options options)
    : CordBackwardWriter(std::forward_as_tuple(), std::move(options)) {}

template <typename Dest>
inline CordBackwardWriter<Dest>::CordBackwardWriter(
    CordBackwardWriter&& that) noexcept
    : CordBackwardWriterBase(static_cast<CordBackwardWriterBase&&>(that)),
      dest_(std::move(that.dest_)) {}

template <typename Dest>
inline CordBackwardWriter<Dest>& CordBackwardWriter<Dest>::operator=(
    CordBackwardWriter&& that) noexcept {
  CordBackwardWriterBase::operator=(
      static_cast<CordBackwardWriterBase&&>(that));
  dest_ = std::move(that.dest_);
  return *this;
}

template <typename Dest>
inline void CordBackwardWriter<Dest>::Reset(Closed) {
  CordBackwardWriterBase::Reset(kClosed);
  dest_.Reset();
}

template <typename Dest>
inline void CordBackwardWriter<Dest>::Reset(const Dest& dest, Options options) {
  CordBackwardWriterBase::Reset(options);
  dest_.Reset(dest);
  Initialize(dest_.get(), options.prepend());
}

template <typename Dest>
inline void CordBackwardWriter<Dest>::Reset(Dest&& dest, Options options) {
  CordBackwardWriterBase::Reset(options);
  dest_.Reset(std::move(dest));
  Initialize(dest_.get(), options.prepend());
}

template <typename Dest>
template <typename... DestArgs>
inline void CordBackwardWriter<Dest>::Reset(std::tuple<DestArgs...> dest_args,
                                            Options options) {
  CordBackwardWriterBase::Reset(options);
  dest_.Reset(std::move(dest_args));
  Initialize(dest_.get(), options.prepend());
}

template <typename Dest>
template <typename DependentDest,
          std::enable_if_t<std::is_same<DependentDest, absl::Cord>::value, int>>
inline void CordBackwardWriter<Dest>::Reset(Options options) {
  Reset(std::forward_as_tuple(), std::move(options));
}

}  // namespace riegeli

#endif  // RIEGELI_BYTES_CORD_BACKWARD_WRITER_H_
