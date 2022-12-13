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

#ifndef _WIN32

// Make `off_t` 64-bit even on 32-bit systems.
#undef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64

#else

#define WIN32_LEAN_AND_MEAN

#endif

#include "riegeli/bytes/fd_mmap_reader.h"

#include <fcntl.h>
#ifdef _WIN32
#include <io.h>
#endif
#include <stddef.h>
#ifdef _WIN32
#include <stdint.h>
#endif
#include <stdio.h>
#ifndef _WIN32
#include <sys/mman.h>
#endif
#include <sys/types.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

#include <cerrno>
#include <limits>
#include <memory>
#include <ostream>
#include <string>
#include <tuple>
#include <utility>

#include "absl/base/optimization.h"
#include "absl/status/status.h"
#ifndef _WIN32
#include "absl/status/statusor.h"
#endif
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "riegeli/base/arithmetic.h"
#include "riegeli/base/assert.h"
#include "riegeli/base/chain.h"
#include "riegeli/base/errno_mapping.h"
#ifndef _WIN32
#include "riegeli/base/no_destructor.h"
#endif
#include "riegeli/base/object.h"
#include "riegeli/base/status.h"
#include "riegeli/base/types.h"
#ifdef _WIN32
#include "riegeli/base/unicode.h"
#endif
#include "riegeli/bytes/chain_reader.h"
#include "riegeli/bytes/fd_internal.h"
#include "riegeli/bytes/reader.h"

namespace riegeli {

namespace {

#ifdef _WIN32

struct HandleDeleter {
  void operator()(void* handle) const {
    RIEGELI_CHECK(CloseHandle(reinterpret_cast<HANDLE>(handle)))
        << WindowsErrorToStatus(IntCast<uint32_t>(GetLastError()),
                                "CloseHandle() failed")
               .message();
  }
};

using UniqueHandle = std::unique_ptr<void, HandleDeleter>;

#endif

#ifndef _WIN32

inline absl::StatusOr<Position> GetPageSize() {
  const long page_size = sysconf(_SC_PAGE_SIZE);
  if (ABSL_PREDICT_FALSE(page_size < 0)) {
    return absl::ErrnoToStatus(errno, "sysconf() failed");
  }
  return IntCast<Position>(page_size);
}

#else

inline Position GetPageSize() {
  SYSTEM_INFO system_info;
  GetSystemInfo(&system_info);
  return IntCast<Position>(system_info.dwAllocationGranularity);
}

#endif

class MMapRef {
 public:
  explicit MMapRef(const char* addr) : addr_(addr) {}

  MMapRef(const MMapRef&) = delete;
  MMapRef& operator=(const MMapRef&) = delete;

  void operator()(absl::string_view data) const;
  void DumpStructure(std::ostream& out) const;
  template <typename MemoryEstimator>
  friend void RiegeliRegisterSubobjects(const MMapRef& self,
                                        MemoryEstimator& memory_estimator) {}

 private:
  const char* addr_;
};

void MMapRef::operator()(absl::string_view data) const {
#ifndef _WIN32
  RIEGELI_CHECK_EQ(munmap(const_cast<char*>(addr_),
                          data.size() + PtrDistance(addr_, data.data())),
                   0)
      << absl::ErrnoToStatus(errno, "munmap() failed").message();
#else
  RIEGELI_CHECK(UnmapViewOfFile(addr_))
      << WindowsErrorToStatus(IntCast<uint32_t>(GetLastError()),
                              "UnmapViewOfFile() failed")
             .message();
#endif
}

void MMapRef::DumpStructure(std::ostream& out) const { out << "[mmap] { }"; }

}  // namespace

void FdMMapReaderBase::Initialize(
    int src, absl::optional<std::string>&& assumed_filename,
    absl::optional<Position> independent_pos,
    absl::optional<Position> max_length) {
  RIEGELI_ASSERT_GE(src, 0)
      << "Failed precondition of FdMMapReader: negative file descriptor";
  filename_ = fd_internal::ResolveFilename(src, std::move(assumed_filename));
  InitializePos(src, independent_pos, max_length);
}

int FdMMapReaderBase::OpenFd(absl::string_view filename, int mode) {
#ifndef _WIN32
  RIEGELI_ASSERT((mode & O_ACCMODE) == O_RDONLY || (mode & O_ACCMODE) == O_RDWR)
      << "Failed precondition of FdMMapReader: "
         "mode must include either O_RDONLY or O_RDWR";
#else
  RIEGELI_ASSERT((mode & (_O_RDONLY | _O_WRONLY | _O_RDWR)) == _O_RDONLY ||
                 (mode & (_O_RDONLY | _O_WRONLY | _O_RDWR)) == _O_RDWR)
      << "Failed precondition of FdMMapReader: "
         "mode must include either _O_RDONLY or _O_RDWR";
#endif
  // TODO: When `absl::string_view` becomes C++17 `std::string_view`:
  // `filename_ = filename`
  filename_.assign(filename.data(), filename.size());
#ifndef _WIN32
again:
  const int src = open(filename_.c_str(), mode, 0666);
  if (ABSL_PREDICT_FALSE(src < 0)) {
    if (errno == EINTR) goto again;
    FailOperation("open()");
    return -1;
  }
#else
  std::wstring filename_wide;
  if (ABSL_PREDICT_FALSE(!Utf8ToWide(filename_, filename_wide))) {
    Fail(absl::InvalidArgumentError("Filename not valid UTF-8"));
    return -1;
  }
  int src;
  if (ABSL_PREDICT_FALSE(_wsopen_s(&src, filename_wide.c_str(), mode,
                                   _SH_DENYNO, _S_IREAD) != 0)) {
    FailOperation("_wsopen_s()");
    return -1;
  }
#endif
  return src;
}

void FdMMapReaderBase::InitializePos(int src,
                                     absl::optional<Position> independent_pos,
                                     absl::optional<Position> max_length) {
  Position initial_pos;
  if (independent_pos != absl::nullopt) {
    initial_pos = *independent_pos;
  } else {
    const fd_internal::Offset file_pos = fd_internal::LSeek(src, 0, SEEK_CUR);
    if (ABSL_PREDICT_FALSE(file_pos < 0)) {
      FailOperation(fd_internal::kLSeekFunctionName);
      return;
    }
    initial_pos = IntCast<Position>(file_pos);
  }

  fd_internal::StatInfo stat_info;
  if (ABSL_PREDICT_FALSE(fd_internal::FStat(src, &stat_info) < 0)) {
    FailOperation(fd_internal::kFStatFunctionName);
    return;
  }
  Position base_pos = 0;
  Position length = IntCast<Position>(stat_info.st_size);
  if (max_length != absl::nullopt) {
    base_pos = initial_pos;
    length = UnsignedMin(SaturatingSub(length, initial_pos), *max_length);
  }
  if (independent_pos == absl::nullopt) base_pos_to_sync_ = base_pos;
  if (length == 0) {
    // The `Chain` to read from was not known in `FdMMapReaderBase` constructor.
    // Set it now to empty.
    ChainReader::Reset(std::forward_as_tuple());
    return;
  }

  Position rounded_base_pos = base_pos;
  if (rounded_base_pos > 0) {
#ifndef _WIN32
    static const NoDestructor<absl::StatusOr<Position>> kPageSize(
        GetPageSize());
    if (ABSL_PREDICT_FALSE(!kPageSize->ok())) {
      Fail(kPageSize->status());
      return;
    }
    rounded_base_pos &= ~(**kPageSize - 1);
#else
    static const Position kPageSize = GetPageSize();
    rounded_base_pos &= ~(kPageSize - 1);
#endif
  }
  const Position rounding = base_pos - rounded_base_pos;
  const Position rounded_length = length + rounding;
  if (ABSL_PREDICT_FALSE(rounded_length > std::numeric_limits<size_t>::max())) {
    Fail(absl::OutOfRangeError("File too large for memory mapping"));
    return;
  }
#ifndef _WIN32
  void* const addr = mmap(nullptr, IntCast<size_t>(rounded_length), PROT_READ,
                          MAP_SHARED, src, IntCast<off_t>(rounded_base_pos));
  if (ABSL_PREDICT_FALSE(addr == MAP_FAILED)) {
    FailOperation("mmap()");
    return;
  }
#else
  const HANDLE file_handle = reinterpret_cast<HANDLE>(_get_osfhandle(src));
  if (ABSL_PREDICT_FALSE(file_handle == INVALID_HANDLE_VALUE ||
                         file_handle == reinterpret_cast<HANDLE>(-2))) {
    FailWindowsOperation("_get_osfhandle()");
    return;
  }
  UniqueHandle memory_handle(reinterpret_cast<void*>(
      CreateFileMappingW(file_handle, nullptr, PAGE_READONLY, 0, 0, nullptr)));
  if (ABSL_PREDICT_FALSE(memory_handle == nullptr)) {
    FailWindowsOperation("CreateFileMappingW()");
    return;
  }
  void* const addr =
      MapViewOfFile(reinterpret_cast<HANDLE>(memory_handle.get()),
                    FILE_MAP_READ, IntCast<DWORD>(rounded_base_pos >> 32),
                    IntCast<DWORD>(rounded_base_pos & 0xffffffff),
                    IntCast<size_t>(rounded_length));
  if (ABSL_PREDICT_FALSE(addr == nullptr)) {
    FailWindowsOperation("MapViewOfFile()");
    return;
  }
#endif

  // The `Chain` to read from was not known in `FdMMapReaderBase` constructor.
  // Set it now.
  ChainReader::Reset(std::forward_as_tuple(ChainBlock::FromExternal<MMapRef>(
      std::forward_as_tuple(static_cast<const char*>(addr)),
      absl::string_view(static_cast<const char*>(addr) + rounding,
                        IntCast<size_t>(length)))));
  if (max_length == absl::nullopt) Seek(initial_pos);
}

void FdMMapReaderBase::InitializeWithExistingData(int src,
                                                  absl::string_view filename,
                                                  const Chain& data) {
  // TODO: When `absl::string_view` becomes C++17 `std::string_view`:
  // `filename_ = filename`.
  filename_.assign(filename.data(), filename.size());
  ChainReader::Reset(data);
}

void FdMMapReaderBase::Done() {
  FdMMapReaderBase::SyncImpl(SyncType::kFromObject);
  ChainReader::Done();
  ChainReader::src().Clear();
}

bool FdMMapReaderBase::FailOperation(absl::string_view operation) {
  const int error_number = errno;
  RIEGELI_ASSERT_NE(error_number, 0)
      << "Failed precondition of FdMMapReaderBase::FailOperation(): "
         "zero errno";
  return Fail(
      absl::ErrnoToStatus(error_number, absl::StrCat(operation, " failed")));
}

#ifdef _WIN32

bool FdMMapReaderBase::FailWindowsOperation(absl::string_view operation) {
  const DWORD error_number = GetLastError();
  RIEGELI_ASSERT_NE(error_number, 0)
      << "Failed precondition of FdMMapReaderBase::FailWindowsOperation(): "
         "zero error code";
  return Fail(WindowsErrorToStatus(IntCast<uint32_t>(error_number),
                                   absl::StrCat(operation, " failed")));
}

#endif

absl::Status FdMMapReaderBase::AnnotateStatusImpl(absl::Status status) {
  if (!filename_.empty()) {
    status = Annotate(status, absl::StrCat("reading ", filename_));
  }
  return ChainReader::AnnotateStatusImpl(std::move(status));
}

bool FdMMapReaderBase::SyncImpl(SyncType sync_type) {
  if (ABSL_PREDICT_FALSE(!ok())) return false;
  const int src = SrcFd();
  if (base_pos_to_sync_ != absl::nullopt) {
    if (ABSL_PREDICT_FALSE(
            fd_internal::LSeek(
                src, IntCast<fd_internal::Offset>(*base_pos_to_sync_ + pos()),
                SEEK_SET) < 0)) {
      return FailOperation(fd_internal::kLSeekFunctionName);
    }
  }
  return true;
}

std::unique_ptr<Reader> FdMMapReaderBase::NewReaderImpl(Position initial_pos) {
  if (ABSL_PREDICT_FALSE(!ok())) return nullptr;
  // `NewReaderImpl()` is thread-safe from this point.
  const int src = SrcFd();
  std::unique_ptr<FdMMapReader<UnownedFd>> reader =
      std::make_unique<FdMMapReader<UnownedFd>>(kClosed);
  reader->InitializeWithExistingData(src, filename(), ChainReader::src());
  reader->Seek(initial_pos);
  return reader;
}

}  // namespace riegeli
