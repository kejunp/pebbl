/**
 * @file environment.hpp
 * @brief Environment system for variable storage and scope management
 */

#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "object.hpp"

class Tracer;

/**
 * @brief Environment for storing variables and managing scopes
 * 
 * The Environment class manages variable storage with lexical scoping.
 * Each environment can have a parent environment, forming a scope chain
 * for variable resolution.
 */
class Environment {
public:
  /**
   * @brief Create a new environment
   * @param parent Optional parent environment for scope chaining
   */
  explicit Environment(std::shared_ptr<Environment> parent = nullptr);

  /**
   * @brief Define a new variable in this environment
   * @param name Variable name
   * @param value Variable value
   * @param is_mutable Whether the variable can be reassigned
   */
  void define(const std::string& name, PEBBLObject value, bool is_mutable = true);

  /**
   * @brief Get a variable's value by name
   * @param name Variable name
   * @return The variable's value
   * @throws std::runtime_error if variable is not found
   */
  PEBBLObject get(const std::string& name) const;

  /**
   * @brief Set a variable's value by name
   * @param name Variable name  
   * @param value New value
   * @throws std::runtime_error if variable is not found or immutable
   */
  void set(const std::string& name, PEBBLObject value);

  /**
   * @brief Check if a variable exists in this environment or parent scopes
   * @param name Variable name
   * @return True if variable exists
   */
  bool exists(const std::string& name) const;

  /**
   * @brief Get the parent environment
   * @return Shared pointer to parent environment (may be null)
   */
  std::shared_ptr<Environment> get_parent() const { return parent_; }
  
  /**
   * @brief Trace all GC objects in this environment
   * @param tracer GC tracer to mark objects
   */
  void trace_objects(class Tracer& tracer) const;

private:
  struct Variable {
    PEBBLObject value;
    bool is_mutable;
    
    Variable(PEBBLObject val, bool mut) : value(val), is_mutable(mut) {}
  };

  std::shared_ptr<Environment> parent_;
  std::unordered_map<std::string, Variable> variables_;
};