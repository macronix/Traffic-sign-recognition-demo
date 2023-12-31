/* Copyright 2019 Google LLC. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "ruy/system_aligned_alloc.h"

#include <cstddef>
#include <cstdlib>

#ifdef _WIN32
#include <malloc.h>
#endif

#if defined RUY_MCU && !defined __ICCARM__ && !defined __ARMCC_VERSION
#include <malloc.h>
#define RUY_USE_MEMALIGN
#endif

namespace ruy {

namespace detail {

void *SystemAlignedAlloc(std::ptrdiff_t num_bytes) {
#ifdef _WIN32
  return _aligned_malloc(num_bytes, kMinimumBlockAlignment);
#elif defined __ICCARM__
  return ::aligned_alloc(kMinimumBlockAlignment, num_bytes);
#elif defined RUY_USE_MEMALIGN
  return memalign(kMinimumBlockAlignment, num_bytes);
#else
  void *ptr;
  if (posix_memalign(&ptr, kMinimumBlockAlignment, num_bytes)) {
    return nullptr;
  }
  return ptr;
#endif
}

void SystemAlignedFree(void *ptr) {
#ifdef _WIN32
  _aligned_free(ptr);
#else
  free(ptr);
#endif
}

}  // namespace detail

}  // namespace ruy
