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

#include "value.h"

#include "memutils.h"

void init_value_array(ValueArray* value_array) {
  value_array->capacity = 0;
  value_array->count = 0;
  value_array->values = NULL;
}

void write_value_array(ValueArray* value_array, Value value) {
  if (value_array->capacity <= value_array->count) {
    int old_capacity = value_array->capacity;
    value_array->capacity = GROW_CAPACITY(old_capacity);
    value_array->values = GROW_ARRAY(Value, value_array->values, value_array->capacity);
  }
  value_array->values[value_array->count++] = value;
}

void free_value_array(ValueArray* value_array) {
  FREE_ARRAY(Value, value_array->values);
  init_value_array(value_array);
}

void print_value(Value value) {
  printf("%g", value);
}