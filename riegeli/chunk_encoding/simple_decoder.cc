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

#include "riegeli/chunk_encoding/simple_decoder.h"

#include <stddef.h>
#include <stdint.h>

#include <limits>
#include <tuple>
#include <vector>

#include "absl/base/optimization.h"
#include "absl/status/status.h"
#include "riegeli/base/arithmetic.h"
#include "riegeli/base/object.h"
#include "riegeli/bytes/limiting_reader.h"
#include "riegeli/bytes/reader.h"
#include "riegeli/chunk_encoding/constants.h"
#include "riegeli/chunk_encoding/decompressor.h"
#include "riegeli/varint/varint_reading.h"

namespace riegeli {

void SimpleDecoder::Done() {
  if (ABSL_PREDICT_FALSE(!values_decompressor_.Close())) {
    Fail(values_decompressor_.status());
  }
}

bool SimpleDecoder::Decode(Reader* src, uint64_t num_records,
                           uint64_t decoded_data_size,
                           std::vector<size_t>& limits) {
  Object::Reset();
  if (ABSL_PREDICT_FALSE(num_records > limits.max_size())) {
    return Fail(absl::ResourceExhaustedError("Too many records"));
  }
  if (ABSL_PREDICT_FALSE(decoded_data_size >
                         std::numeric_limits<size_t>::max())) {
    return Fail(absl::ResourceExhaustedError("Records too large"));
  }

  uint8_t compression_type_byte;
  if (ABSL_PREDICT_FALSE(!src->ReadByte(compression_type_byte))) {
    return Fail(src->StatusOrAnnotate(
        absl::InvalidArgumentError("Reading compression type failed")));
  }
  const CompressionType compression_type =
      static_cast<CompressionType>(compression_type_byte);

  uint64_t sizes_size;
  if (ABSL_PREDICT_FALSE(!ReadVarint64(*src, sizes_size))) {
    return Fail(src->StatusOrAnnotate(
        absl::InvalidArgumentError("Reading size of sizes failed")));
  }

  chunk_encoding_internal::Decompressor<LimitingReader<>> sizes_decompressor(
      std::forward_as_tuple(
          src, LimitingReaderBase::Options().set_exact_length(sizes_size)),
      compression_type);
  if (ABSL_PREDICT_FALSE(!sizes_decompressor.ok())) {
    return Fail(sizes_decompressor.status());
  }
  limits.clear();
  size_t limit = 0;
  while (limits.size() != num_records) {
    uint64_t size;
    if (ABSL_PREDICT_FALSE(!ReadVarint64(sizes_decompressor.reader(), size))) {
      return Fail(sizes_decompressor.reader().StatusOrAnnotate(
          absl::InvalidArgumentError("Reading record size failed")));
    }
    if (ABSL_PREDICT_FALSE(size > decoded_data_size - limit)) {
      return Fail(
          absl::InvalidArgumentError("Decoded data size larger than expected"));
    }
    limit += IntCast<size_t>(size);
    limits.push_back(limit);
  }
  if (ABSL_PREDICT_FALSE(!sizes_decompressor.VerifyEndAndClose())) {
    return Fail(sizes_decompressor.status());
  }
  if (ABSL_PREDICT_FALSE(limit != decoded_data_size)) {
    return Fail(
        absl::InvalidArgumentError("Decoded data size smaller than expected"));
  }

  values_decompressor_.Reset(src, compression_type);
  if (ABSL_PREDICT_FALSE(!values_decompressor_.ok())) {
    return Fail(values_decompressor_.status());
  }
  return true;
}

bool SimpleDecoder::VerifyEndAndClose() {
  values_decompressor_.VerifyEnd();
  return Close();
}

}  // namespace riegeli
