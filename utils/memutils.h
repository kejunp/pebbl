/*
   Copyright 2025 Kejun Pan

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

#include "common.h"

#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(type, ptr, new_count) (type*)Realloc(ptr, sizeof(type) * new_count)

#define FREE_ARRAY(type, ptr) Realloc(ptr, 0)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Wrapper around realloc
 * @param ptr Pointer to be reallocated
 * @param new_count Size for the pointer to be reallocated
 * @return Reallocated pointer
 */
static inline void* Realloc(void* ptr, size_t new_count) {
  if (new_count == 0) {
    free(ptr);
    return NULL;
  }
  void* result = realloc(ptr, new_count);
  if (result == NULL) exit(1);
  return result;
}

#ifdef __cplusplus
}
#endif