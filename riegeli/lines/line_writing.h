// Copyright 2020 Google LLC
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

#ifndef RIEGELI_LINES_LINE_WRITING_H_
#define RIEGELI_LINES_LINE_WRITING_H_

#include <stddef.h>

#include <tuple>
#include <type_traits>
#include <utility>

#include "absl/base/attributes.h"
#include "absl/base/optimization.h"
#include "absl/meta/type_traits.h"
#include "riegeli/base/assert.h"
#include "riegeli/base/type_traits.h"
#include "riegeli/bytes/writer.h"
#include "riegeli/lines/newline.h"

namespace riegeli {

// Options for `WriteLine()`.
class WriteLineOptions {
 public:
  WriteLineOptions() noexcept {}

  // Line terminator representation to write.
  //
  // Default: `WriteNewline::kNative`.
  WriteLineOptions& set_newline(WriteNewline newline) & {
    newline_ = newline;
    return *this;
  }
  WriteLineOptions&& set_newline(WriteNewline newline) && {
    return std::move(set_newline(newline));
  }
  WriteNewline newline() const { return newline_; }

 private:
  WriteNewline newline_ = WriteNewline::kNative;
};

// Writes an optional string, then a line terminator.
//
// `std::string&&` is accepted with a template to avoid implicit conversions
// to `std::string` which can be ambiguous against `absl::string_view`
// (e.g. `const char*`).
//
// Return values:
//  * `true`  - success
//  * `false` - failure (`!ok()`)
template <typename... Args,
          std::enable_if_t<
              absl::conjunction<
                  std::is_convertible<GetTypeFromEndT<1, Args&&...>, Writer&>,
                  TupleElementsSatisfy<RemoveTypesFromEndT<1, Args&&...>,
                                       IsStringifiable>>::value,
              int> = 0>
bool WriteLine(Args&&... args);
template <typename... Args,
          std::enable_if_t<
              absl::conjunction<
                  std::is_convertible<GetTypeFromEndT<1, Args&&...>,
                                      WriteLineOptions>,
                  std::is_convertible<GetTypeFromEndT<2, Args&&...>, Writer&>,
                  TupleElementsSatisfy<RemoveTypesFromEndT<2, Args&&...>,
                                       IsStringifiable>>::value,
              int> = 0>
bool WriteLine(Args&&... args);

// Implementation details follow.

namespace write_line_internal {

template <typename... Srcs, size_t... indices>
ABSL_ATTRIBUTE_ALWAYS_INLINE inline bool WriteLineInternal(
    ABSL_ATTRIBUTE_UNUSED std::tuple<Srcs...> srcs, Writer& dest,
    WriteLineOptions options, std::index_sequence<indices...>) {
  if (ABSL_PREDICT_FALSE(
          !dest.Write(std::forward<Srcs>(std::get<indices>(srcs))...))) {
    return false;
  }
  switch (options.newline()) {
    case WriteNewline::kLf:
      return dest.Write('\n');
    case WriteNewline::kCr:
      return dest.Write('\r');
    case WriteNewline::kCrLf:
      return dest.Write("\r\n");
  }
  RIEGELI_ASSERT_UNREACHABLE()
      << "Unknown newline: " << static_cast<int>(options.newline());
}

}  // namespace write_line_internal

template <typename... Args,
          std::enable_if_t<
              absl::conjunction<
                  std::is_convertible<GetTypeFromEndT<1, Args&&...>, Writer&>,
                  TupleElementsSatisfy<RemoveTypesFromEndT<1, Args&&...>,
                                       IsStringifiable>>::value,
              int>>
ABSL_ATTRIBUTE_ALWAYS_INLINE inline bool WriteLine(Args&&... args) {
  return write_line_internal::WriteLineInternal(
      RemoveFromEnd<1>(std::forward<Args>(args)...),
      GetFromEnd<1>(std::forward<Args>(args)...), WriteLineOptions(),
      std::make_index_sequence<sizeof...(Args) - 1>());
}

template <typename... Args,
          std::enable_if_t<
              absl::conjunction<
                  std::is_convertible<GetTypeFromEndT<1, Args&&...>,
                                      WriteLineOptions>,
                  std::is_convertible<GetTypeFromEndT<2, Args&&...>, Writer&>,
                  TupleElementsSatisfy<RemoveTypesFromEndT<2, Args&&...>,
                                       IsStringifiable>>::value,
              int>>
ABSL_ATTRIBUTE_ALWAYS_INLINE inline bool WriteLine(Args&&... args) {
  return write_line_internal::WriteLineInternal(
      RemoveFromEnd<2>(std::forward<Args>(args)...),
      GetFromEnd<2>(std::forward<Args>(args)...),
      GetFromEnd<1>(std::forward<Args>(args)...),
      std::make_index_sequence<sizeof...(Args) - 2>());
}

}  // namespace riegeli

#endif  // RIEGELI_LINES_LINE_WRITING_H_
