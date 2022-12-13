// Copyright 2018 Google LLC
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

#ifndef RIEGELI_BYTES_ARRAY_WRITER_H_
#define RIEGELI_BYTES_ARRAY_WRITER_H_

#include <stddef.h>

#include <tuple>
#include <type_traits>
#include <utility>

#include "absl/base/attributes.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "riegeli/base/dependency.h"
#include "riegeli/base/object.h"
#include "riegeli/base/types.h"
#include "riegeli/bytes/pushable_writer.h"
#include "riegeli/bytes/writer.h"

namespace riegeli {

class Reader;
template <typename Src>
class StringReader;

// Template parameter independent part of `ArrayWriter`.
class ArrayWriterBase : public PushableWriter {
 public:
  // Returns the array being written to. Unchanged by `Close()`.
  virtual absl::Span<char> DestSpan() = 0;
  virtual absl::Span<const char> DestSpan() const = 0;

  // Returns written data in a prefix of the original array. Valid only after
  // `Close()` or `Flush()`.
  absl::Span<char> written() { return written_; }
  absl::Span<const char> written() const { return written_; }

  bool PrefersCopying() const override { return true; }
  bool SupportsRandomAccess() override { return true; }
  bool SupportsReadMode() override { return true; }

 protected:
  using PushableWriter::PushableWriter;

  ArrayWriterBase(ArrayWriterBase&& that) noexcept;
  ArrayWriterBase& operator=(ArrayWriterBase&& that) noexcept;

  void Reset(Closed);
  void Reset();
  void Initialize(absl::Span<char> dest);
  void set_written(absl::Span<char> written) { written_ = written; }

  void Done() override;
  bool PushBehindScratch(size_t recommended_length) override;
  bool FlushBehindScratch(FlushType flush_type) override;
  bool SeekBehindScratch(Position new_pos) override;
  absl::optional<Position> SizeBehindScratch() override;
  bool TruncateBehindScratch(Position new_size) override;
  Reader* ReadModeBehindScratch(Position initial_pos) override;

 private:
  // Written data. Valid only after `Close()` or `Flush()`.
  //
  // Size of written data is always `UnsignedMax(pos(), written_.size())`.
  // This is used to determine the size after seeking backwards.
  absl::Span<char> written_;

  AssociatedReader<StringReader<absl::string_view>> associated_reader_;

  // Invariant: `start_pos() == 0`
};

// A `Writer` which writes to a preallocated array with a known size limit.
//
// It supports `ReadMode()`.
//
// The `Dest` template parameter specifies the type of the object providing and
// possibly owning the array being written to. `Dest` must support
// `Dependency<absl::Span<char>, Dest>`, e.g.
// `absl::Span<char>` (not owned, default), `std::string*` (not owned),
// `std::string` (owned).
//
// By relying on CTAD the template argument can be deduced as the value type of
// the first constructor argument, except that CTAD is deleted if the first
// constructor argument is a reference to a type that `absl::Span<char>` would
// be constructible from, other than `absl::Span<char>` itself (to avoid writing
// to an unintentionally separate copy of an existing object). This requires
// C++17.
//
// The array must not be destroyed until the `ArrayWriter` is closed or no
// longer used.
template <typename Dest = absl::Span<char>>
class ArrayWriter : public ArrayWriterBase {
 public:
  // Creates a closed `ArrayWriter`.
  explicit ArrayWriter(Closed) noexcept : ArrayWriterBase(kClosed) {}

  // Will write to the array provided by `dest`.
  explicit ArrayWriter(const Dest& dest);
  explicit ArrayWriter(Dest&& dest);

  // Will write to the array provided by a `Dest` constructed from elements of
  // `dest_args`. This avoids constructing a temporary `Dest` and moving from
  // it.
  template <typename... DestArgs>
  explicit ArrayWriter(std::tuple<DestArgs...> dest_args);

  // Will write to `absl::MakeSpan(dest, size)`. This constructor is present
  // only if `Dest` is `absl::Span<char>`.
  template <typename DependentDest = Dest,
            std::enable_if_t<
                std::is_same<DependentDest, absl::Span<char>>::value, int> = 0>
  explicit ArrayWriter(char* dest, size_t size);

  ArrayWriter(ArrayWriter&& that) noexcept;
  ArrayWriter& operator=(ArrayWriter&& that) noexcept;

  // Makes `*this` equivalent to a newly constructed `ArrayWriter`. This avoids
  // constructing a temporary `ArrayWriter` and moving from it.
  ABSL_ATTRIBUTE_REINITIALIZES void Reset(Closed);
  ABSL_ATTRIBUTE_REINITIALIZES void Reset(const Dest& dest);
  ABSL_ATTRIBUTE_REINITIALIZES void Reset(Dest&& dest);
  template <typename... DestArgs>
  ABSL_ATTRIBUTE_REINITIALIZES void Reset(std::tuple<DestArgs...> dest_args);
  template <typename DependentDest = Dest,
            std::enable_if_t<
                std::is_same<DependentDest, absl::Span<char>>::value, int> = 0>
  ABSL_ATTRIBUTE_REINITIALIZES void Reset(char* dest, size_t size);

  // Returns the object providing and possibly owning the array being written
  // to. Unchanged by `Close()`.
  Dest& dest() { return dest_.manager(); }
  const Dest& dest() const { return dest_.manager(); }
  absl::Span<char> DestSpan() override { return dest_.get(); }
  absl::Span<const char> DestSpan() const override { return dest_.get(); }

 private:
  void MoveDest(ArrayWriter&& that);

  // The object providing and possibly owning the array being written to.
  Dependency<absl::Span<char>, Dest> dest_;
};

// Support CTAD.
#if __cpp_deduction_guides
explicit ArrayWriter(Closed)->ArrayWriter<DeleteCtad<Closed>>;
template <typename Dest>
explicit ArrayWriter(const Dest& dest) -> ArrayWriter<std::conditional_t<
    !std::is_same<std::decay_t<Dest>, absl::Span<char>>::value &&
        std::is_constructible<absl::Span<char>, const Dest&>::value &&
        !std::is_pointer<Dest>::value,
    DeleteCtad<Dest&&>, std::decay_t<Dest>>>;
template <typename Dest>
explicit ArrayWriter(Dest&& dest) -> ArrayWriter<std::conditional_t<
    !std::is_same<std::decay_t<Dest>, absl::Span<char>>::value &&
        std::is_lvalue_reference<Dest>::value &&
        std::is_constructible<absl::Span<char>, Dest>::value &&
        !std::is_pointer<std::remove_reference_t<Dest>>::value,
    DeleteCtad<Dest&&>, std::decay_t<Dest>>>;
template <typename... DestArgs>
explicit ArrayWriter(std::tuple<DestArgs...> dest_args)
    -> ArrayWriter<DeleteCtad<std::tuple<DestArgs...>>>;
explicit ArrayWriter(char* dest, size_t size)->ArrayWriter<absl::Span<char>>;
#endif

// Implementation details follow.

inline ArrayWriterBase::ArrayWriterBase(ArrayWriterBase&& that) noexcept
    : PushableWriter(static_cast<PushableWriter&&>(that)),
      written_(that.written_),
      associated_reader_(std::move(that.associated_reader_)) {}

inline ArrayWriterBase& ArrayWriterBase::operator=(
    ArrayWriterBase&& that) noexcept {
  PushableWriter::operator=(static_cast<PushableWriter&&>(that));
  written_ = that.written_;
  associated_reader_ = std::move(that.associated_reader_);
  return *this;
}

inline void ArrayWriterBase::Reset(Closed) {
  PushableWriter::Reset(kClosed);
  written_ = absl::Span<char>();
  associated_reader_.Reset();
}

inline void ArrayWriterBase::Reset() {
  PushableWriter::Reset();
  written_ = absl::Span<char>();
  associated_reader_.Reset();
}

inline void ArrayWriterBase::Initialize(absl::Span<char> dest) {
  set_buffer(dest.data(), dest.size());
}

template <typename Dest>
inline ArrayWriter<Dest>::ArrayWriter(const Dest& dest) : dest_(dest) {
  Initialize(dest_.get());
}

template <typename Dest>
inline ArrayWriter<Dest>::ArrayWriter(Dest&& dest) : dest_(std::move(dest)) {
  Initialize(dest_.get());
}

template <typename Dest>
template <typename... DestArgs>
inline ArrayWriter<Dest>::ArrayWriter(std::tuple<DestArgs...> dest_args)
    : dest_(std::move(dest_args)) {
  Initialize(dest_.get());
}

template <typename Dest>
template <
    typename DependentDest,
    std::enable_if_t<std::is_same<DependentDest, absl::Span<char>>::value, int>>
inline ArrayWriter<Dest>::ArrayWriter(char* dest, size_t size)
    : ArrayWriter(absl::MakeSpan(dest, size)) {}

template <typename Dest>
inline ArrayWriter<Dest>::ArrayWriter(ArrayWriter&& that) noexcept
    : ArrayWriterBase(static_cast<ArrayWriterBase&&>(that)) {
  MoveDest(std::move(that));
}

template <typename Dest>
inline ArrayWriter<Dest>& ArrayWriter<Dest>::operator=(
    ArrayWriter&& that) noexcept {
  ArrayWriterBase::operator=(static_cast<ArrayWriterBase&&>(that));
  MoveDest(std::move(that));
  return *this;
}

template <typename Dest>
inline void ArrayWriter<Dest>::Reset(Closed) {
  ArrayWriterBase::Reset(kClosed);
  dest_.Reset();
}

template <typename Dest>
inline void ArrayWriter<Dest>::Reset(const Dest& dest) {
  ArrayWriterBase::Reset();
  dest_.Reset(dest);
  Initialize(dest_.get());
}

template <typename Dest>
inline void ArrayWriter<Dest>::Reset(Dest&& dest) {
  ArrayWriterBase::Reset();
  dest_.Reset(std::move(dest));
  Initialize(dest_.get());
}

template <typename Dest>
template <typename... DestArgs>
inline void ArrayWriter<Dest>::Reset(std::tuple<DestArgs...> dest_args) {
  ArrayWriterBase::Reset();
  dest_.Reset(std::move(dest_args));
  Initialize(dest_.get());
}

template <typename Dest>
template <
    typename DependentDest,
    std::enable_if_t<std::is_same<DependentDest, absl::Span<char>>::value, int>>
inline void ArrayWriter<Dest>::Reset(char* dest, size_t size) {
  Reset(absl::MakeSpan(dest, size));
}

template <typename Dest>
inline void ArrayWriter<Dest>::MoveDest(ArrayWriter&& that) {
  if (dest_.kIsStable) {
    dest_ = std::move(that.dest_);
  } else {
    BehindScratch behind_scratch(this);
    const size_t cursor_index = start_to_cursor();
    const size_t written_size = written().size();
    dest_ = std::move(that.dest_);
    if (start() != nullptr) {
      set_buffer(dest_.get().data(), dest_.get().size(), cursor_index);
    }
    if (written().data() != nullptr) {
      set_written(absl::MakeSpan(dest_.get().data(), written_size));
    }
  }
}

}  // namespace riegeli

#endif  // RIEGELI_BYTES_ARRAY_WRITER_H_
