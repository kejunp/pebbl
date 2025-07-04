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

#ifdef __cplusplus
extern "C" {
#endif

typedef double Value;

typedef struct {
  size_t capacity;
  size_t count;     
  Value* values;   // Array, not pointer!
} ValueArray;

/**
 * @brief Initializes a ValueArray struct
 * @param value_array Pointer to an uninitialized ValueArray
 */
void init_value_array(ValueArray* value_array);

/**
 * @brief Writes a value to a ValueArray struct
 * @param value_array Array to be appended on
 * @param new_value Value to be inserted into the array
 */
void write_value_array(ValueArray* value_array, Value new_value);

/**
 * @brief Deallocates any allocated memory and resets struct
 * @param value_array Array to be freed (struct will be reset)
 */
void free_value_array(ValueArray* value_array);

/**
 * @brief Prints a Value
 * @param value Value to be printed
 */
void print_value(Value value);

#ifdef __cplusplus
}
#endif