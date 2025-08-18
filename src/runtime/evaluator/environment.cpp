/**
 * @file environment.cpp
 * @brief Implementation of the Environment system
 */

#include "environment.hpp"

#include <stdexcept>

#include "gc.hpp"

Environment::Environment(std::shared_ptr<Environment> parent) : parent_(parent) {
}

void Environment::define(const std::string& name, PEBBLObject value, bool is_mutable) {
  variables_.emplace(name, Variable(value, is_mutable));
}

PEBBLObject Environment::get(const std::string& name) const {
  auto it = variables_.find(name);
  if (it != variables_.end()) {
    return it->second.value;
  }

  if (parent_) {
    return parent_->get(name);
  }

  throw std::runtime_error("Undefined variable '" + name + "'");
}

void Environment::set(const std::string& name, PEBBLObject value) {
  auto it = variables_.find(name);
  if (it != variables_.end()) {
    if (!it->second.is_mutable) {
      throw std::runtime_error("Cannot assign to immutable variable '" + name + "'");
    }
    it->second.value = value;
    return;
  }

  if (parent_) {
    parent_->set(name, value);
    return;
  }

  throw std::runtime_error("Undefined variable '" + name + "'");
}

bool Environment::exists(const std::string& name) const {
  if (variables_.count(name) > 0) {
    return true;
  }

  if (parent_) {
    return parent_->exists(name);
  }

  return false;
}

void Environment::trace_objects(Tracer& tracer) const {
  // Trace all GC objects in this environment's variables
  for (const auto& [name, variable] : variables_) {
    if (variable.value.is_gc_ptr()) {
      tracer.mark(variable.value.as_gc_ptr());
    }
  }
}