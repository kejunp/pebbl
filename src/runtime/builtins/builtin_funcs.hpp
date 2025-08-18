/**
 * @file builtin_funcs.hpp
 * @brief Implementation of all PEBBL builtin functions
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "builtin_objects.hpp"

// Forward declaration
class Interpreter;

/**
 * @brief Namespace containing all builtin function implementations
 */
namespace BuiltinFunctions {

/**
 * @brief Print function - prints arguments and returns null
 * @param args Vector of arguments to print
 * @param interp Reference to interpreter for stringify calls
 * @return PEBBLObject null value
 */
inline PEBBLObject print_impl(const std::vector<PEBBLObject>& args, class Interpreter& interp) {
  for (size_t i = 0; i < args.size(); ++i) {
    if (i > 0) std::cout << " ";
    std::cout << interp.stringify(args[i]);
  }
  std::cout << std::endl;
  return PEBBLObject::make_null();
}

/**
 * @brief Length function - returns length of strings, arrays, or dicts
 * @param args Vector containing exactly one argument
 * @param interp Reference to interpreter for error reporting
 * @return PEBBLObject containing length as int32
 */
inline PEBBLObject length_impl(const std::vector<PEBBLObject>& args, class Interpreter& interp) {
  if (args.size() != 1) {
    interp.report_error("length() expects exactly 1 argument, got " + std::to_string(args.size()));
    return PEBBLObject::make_null();
  }

  const auto& obj = args[0];
  if (obj.is_gc_ptr()) {
    auto* gc_obj = obj.as_gc_ptr();
    switch (gc_obj->tag) {
      case GCTag::STRING: {
        auto* str = static_cast<PEBBLString*>(gc_obj);
        return PEBBLObject::make_int32(static_cast<int32_t>(str->length()));
      }
      case GCTag::ARRAY: {
        auto* arr = static_cast<PEBBLArray*>(gc_obj);
        return PEBBLObject::make_int32(static_cast<int32_t>(arr->length()));
      }
      case GCTag::DICT: {
        auto* dict = static_cast<PEBBLDict*>(gc_obj);
        return PEBBLObject::make_int32(static_cast<int32_t>(dict->size()));
      }
      default:
        break;
    }
  }
  interp.report_error("length() can only be called on strings, arrays, or dictionaries");
  return PEBBLObject::make_null();
}

/**
 * @brief Type function - returns the type of an object as a string
 * @param args Vector containing exactly one argument
 * @param interp Reference to interpreter for heap allocation
 * @return PEBBLObject containing type name as string
 */
inline PEBBLObject type_impl(const std::vector<PEBBLObject>& args, class Interpreter& interp) {
  if (args.size() != 1) {
    interp.report_error("type() expects exactly 1 argument, got " + std::to_string(args.size()));
    return PEBBLObject::make_null();
  }

  const auto& obj = args[0];
  std::string type_name;

  if (obj.is_null()) {
    type_name = "null";
  } else if (obj.is_bool()) {
    type_name = "boolean";
  } else if (obj.is_int32()) {
    type_name = "integer";
  } else if (obj.is_double()) {
    type_name = "float";
  } else if (obj.is_gc_ptr()) {
    auto* gc_obj = obj.as_gc_ptr();
    switch (gc_obj->tag) {
      case GCTag::STRING:
        type_name = "string";
        break;
      case GCTag::ARRAY:
        type_name = "array";
        break;
      case GCTag::DICT:
        type_name = "dict";
        break;
      case GCTag::FUNCTION:
        type_name = "function";
        break;
      case GCTag::BUILTIN_FUNCTION:
        type_name = "builtin_function";
        break;
      default:
        type_name = "object";
        break;
    }
  } else {
    type_name = "unknown";
  }

  auto* str_obj = interp.get_heap().allocate<PEBBLString>(type_name);
  return PEBBLObject::make_gc_ptr(str_obj);
}

/**
 * @brief String conversion function - converts values to strings
 * @param args Vector containing exactly one argument
 * @param interp Reference to interpreter for stringify and heap allocation
 * @return PEBBLObject containing string representation
 */
inline PEBBLObject str_impl(const std::vector<PEBBLObject>& args, class Interpreter& interp) {
  if (args.size() != 1) {
    interp.report_error("str() expects exactly 1 argument, got " + std::to_string(args.size()));
    return PEBBLObject::make_null();
  }

  std::string str_value = interp.stringify(args[0]);
  auto* str_obj = interp.get_heap().allocate<PEBBLString>(str_value);
  return PEBBLObject::make_gc_ptr(str_obj);
}

/**
 * @brief Push function - adds element to array
 * @param args Vector containing array and value to add
 * @param interp Reference to interpreter for error reporting
 * @return PEBBLObject null value
 */
inline PEBBLObject push_impl(const std::vector<PEBBLObject>& args, class Interpreter& interp) {
  if (args.size() != 2) {
    interp.report_error("push() expects exactly 2 arguments, got " + std::to_string(args.size()));
    return PEBBLObject::make_null();
  }

  const auto& array_obj = args[0];
  const auto& value = args[1];

  if (!array_obj.is_gc_ptr()) {
    interp.report_error("push() first argument must be an array");
    return PEBBLObject::make_null();
  }

  auto* gc_obj = array_obj.as_gc_ptr();
  if (gc_obj->tag != GCTag::ARRAY) {
    interp.report_error("push() first argument must be an array");
    return PEBBLObject::make_null();
  }

  auto* array = static_cast<PEBBLArray*>(gc_obj);
  array->push(value);
  return PEBBLObject::make_null();
}

/**
 * @brief Pop function - removes and returns last element from array
 * @param args Vector containing exactly one array argument
 * @param interp Reference to interpreter for error reporting
 * @return PEBBLObject containing popped value or null
 */
inline PEBBLObject pop_impl(const std::vector<PEBBLObject>& args, class Interpreter& interp) {
  if (args.size() != 1) {
    interp.report_error("pop() expects exactly 1 argument, got " + std::to_string(args.size()));
    return PEBBLObject::make_null();
  }

  const auto& array_obj = args[0];

  if (!array_obj.is_gc_ptr()) {
    interp.report_error("pop() argument must be an array");
    return PEBBLObject::make_null();
  }

  auto* gc_obj = array_obj.as_gc_ptr();
  if (gc_obj->tag != GCTag::ARRAY) {
    interp.report_error("pop() argument must be an array");
    return PEBBLObject::make_null();
  }

  auto* array = static_cast<PEBBLArray*>(gc_obj);
  return array->pop();
}

}  // namespace BuiltinFunctions