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

#include "riegeli/brotli/brotli_allocator.h"

#include <stddef.h>

namespace riegeli {

namespace brotli_internal {

void* RiegeliBrotliAllocFunc(void* opaque, size_t size) {
  return static_cast<const BrotliAllocator::Interface*>(opaque)->Alloc(size);
}

void RiegeliBrotliFreeFunc(void* opaque, void* ptr) {
  static_cast<const BrotliAllocator::Interface*>(opaque)->Free(ptr);
}

}  // namespace brotli_internal

BrotliAllocator::Interface::~Interface() {}

}  // namespace riegeli
