/**
 * @file builtin_objects.hpp
 * @brief Built-in garbage-collected object types (String, Array, Dict)
 */

#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "gc.hpp"
#include "object.hpp"

// Forward declaration to avoid circular includes
class Interpreter;

/**
 * @brief Garbage-collected string object
 */
class PEBBLString : public GCObject {
public:
  std::string value;

  explicit PEBBLString(const std::string& str) : GCObject(GCTag::STRING), value(str) {
  }

  explicit PEBBLString(std::string&& str) : GCObject(GCTag::STRING), value(std::move(str)) {
  }

  void trace(Tracer& /* tracer */) override {
    // Strings contain no GC references
  }

  std::size_t length() const {
    return value.length();
  }
};

/**
 * @brief Garbage-collected array object
 */
class PEBBLArray : public GCObject {
public:
  std::vector<PEBBLObject> elements;

  explicit PEBBLArray(std::vector<PEBBLObject> elems = {}) :
      GCObject(GCTag::ARRAY), elements(std::move(elems)) {
  }

  void trace(Tracer& tracer) override {
    for (const auto& element : elements) {
      if (element.is_gc_ptr()) {
        tracer.mark(element.as_gc_ptr());
      }
    }
  }

  std::size_t length() const {
    return elements.size();
  }

  PEBBLObject get(std::size_t index) const {
    if (index >= elements.size()) {
      return PEBBLObject::make_null();
    }
    return elements[index];
  }

  void set(std::size_t index, PEBBLObject value) {
    if (index >= elements.size()) {
      elements.resize(index + 1, PEBBLObject::make_null());
    }
    elements[index] = value;
  }

  void push(PEBBLObject value) {
    elements.push_back(value);
  }

  PEBBLObject pop() {
    if (elements.empty()) {
      return PEBBLObject::make_null();
    }
    PEBBLObject value = elements.back();
    elements.pop_back();
    return value;
  }
};

/**
 * @brief Garbage-collected dictionary object
 */
class PEBBLDict : public GCObject {
public:
  std::unordered_map<std::string, PEBBLObject> entries;

  explicit PEBBLDict(std::unordered_map<std::string, PEBBLObject> ents = {}) :
      GCObject(GCTag::DICT), entries(std::move(ents)) {
  }

  void trace(Tracer& tracer) override {
    for (const auto& [key, value] : entries) {
      if (value.is_gc_ptr()) {
        tracer.mark(value.as_gc_ptr());
      }
    }
  }

  std::size_t size() const {
    return entries.size();
  }

  PEBBLObject get(const std::string& key) const {
    auto it = entries.find(key);
    if (it == entries.end()) {
      return PEBBLObject::make_null();
    }
    return it->second;
  }

  void set(const std::string& key, PEBBLObject value) {
    entries[key] = value;
  }

  bool has_key(const std::string& key) const {
    return entries.count(key) > 0;
  }

  bool remove(const std::string& key) {
    return entries.erase(key) > 0;
  }

  std::vector<std::string> keys() const {
    std::vector<std::string> result;
    result.reserve(entries.size());
    for (const auto& [key, value] : entries) {
      result.push_back(key);
    }
    return result;
  }
};

/**
 * @brief Garbage-collected function object
 */
class PEBBLFunction : public GCObject {
public:
  std::string name;
  std::vector<std::string> parameters;
  std::shared_ptr<class Environment> closure;
  const class BlockStatementNode* body;

  PEBBLFunction(
      const std::string& func_name,
      std::vector<std::string> params,
      std::shared_ptr<class Environment> env,
      const class BlockStatementNode* func_body) :
      GCObject(GCTag::FUNCTION), name(func_name), parameters(std::move(params)), closure(env),
      body(func_body) {
  }

  void trace(Tracer& /* tracer */) override {
    // The closure environment is shared_ptr managed
    // The body is owned by the AST, not us
  }

  std::size_t arity() const {
    return parameters.size();
  }
};

/**
 * @brief Native C++ function callable from PEBBL
 */
class PEBBLBuiltinFunction : public GCObject {
public:
  using NativeFn = std::function<PEBBLObject(const std::vector<PEBBLObject>&, Interpreter&)>;

  std::string name;
  size_t arity;
  NativeFn function;

  PEBBLBuiltinFunction(const std::string& func_name, size_t param_count, NativeFn fn) :
      GCObject(GCTag::BUILTIN_FUNCTION), name(func_name), arity(param_count), function(fn) {
  }

  void trace(Tracer& /* tracer */) override {
    // Native functions contain no GC references
  }
};