// Copyright 2022 Google LLC
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

#include "riegeli/messages/text_parse.h"

#include <memory>
#include <string>

#include "absl/base/optimization.h"
#include "absl/status/status.h"
#include "absl/strings/cord.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/io/tokenizer.h"
#include "google/protobuf/message.h"
#include "google/protobuf/text_format.h"
#include "riegeli/base/chain.h"
#include "riegeli/bytes/chain_reader.h"
#include "riegeli/bytes/cord_reader.h"
#include "riegeli/bytes/reader.h"
#include "riegeli/bytes/string_reader.h"
#include "riegeli/messages/message_parse.h"

namespace riegeli {

namespace messages_internal {

void StringErrorCollector::AddError(int line,
                                    google::protobuf::io::ColumnNumber column,
                                    const std::string& message) {
  if (!errors_.empty()) {
    absl::StrAppend(&errors_, errors_.back() == '.' ? " " : ". ");
  }
  if (line >= 0) {
    absl::StrAppendFormat(&errors_, "At %d:%d: ", line + 1, column + 1);
  }
  absl::StrAppend(&errors_, message);
}

}  // namespace messages_internal

TextParseOptions::TextParseOptions()
    : error_collector_(
          std::make_unique<messages_internal::StringErrorCollector>()) {
  parser_.RecordErrorsTo(error_collector_.get());
}

namespace messages_internal {

absl::Status TextParseFromReaderImpl(Reader& src,
                                     google::protobuf::Message& dest,
                                     const TextParseOptions& options) {
  ReaderInputStream input_stream(&src);
  google::protobuf::TextFormat::Parser parser = options.parser();
  const bool parse_ok = options.merge() ? parser.Merge(&input_stream, &dest)
                                        : parser.Parse(&input_stream, &dest);
  if (ABSL_PREDICT_FALSE(!src.ok())) return src.status();
  if (ABSL_PREDICT_FALSE(!parse_ok)) {
    std::string message = absl::StrCat("Failed to text-parse message of type ",
                                       dest.GetTypeName());
    if (!options.error_collector_->errors().empty()) {
      absl::StrAppend(&message, ". ", options.error_collector_->errors());
    }
    return src.AnnotateStatus(absl::InvalidArgumentError(message));
  }
  return absl::OkStatus();
}

}  // namespace messages_internal

absl::Status TextParseFromString(absl::string_view src,
                                 google::protobuf::Message& dest,
                                 const TextParseOptions& options) {
  return TextParseFromReader(StringReader<>(src), dest, options);
}

absl::Status TextParseFromChain(const Chain& src,
                                google::protobuf::Message& dest,
                                const TextParseOptions& options) {
  return TextParseFromReader(ChainReader<>(&src), dest, options);
}

absl::Status TextParseFromCord(const absl::Cord& src,
                               google::protobuf::Message& dest,
                               const TextParseOptions& options) {
  return TextParseFromReader(CordReader<>(&src), dest, options);
}

}  // namespace riegeli
