/**
 * @file interpreter.cpp
 * @brief Implementation of the main interpreter
 */

#include "interpreter.hpp"

#include <iostream>
#include <sstream>

#include "builtin_funcs.hpp"
#include "builtin_objects.hpp"
#include "compiler.hpp"
#include "vm.hpp"

Interpreter::Interpreter(GCHeap& heap, bool use_bytecode) 
    : heap_(heap), use_bytecode_(use_bytecode) {
  global_env_ = std::make_shared<Environment>();
  current_env_ = global_env_;

  // Register this interpreter as a GC root tracer
  heap_.add_root_tracer([this](Tracer& tracer) { this->trace_roots(tracer); });

  // Initialize bytecode components if requested
  if (use_bytecode_) {
    compiler_ = std::make_unique<Compiler>(heap_);
    vm_ = std::make_unique<VM>(heap_);
  }

  register_builtin_functions();
}

PEBBLObject Interpreter::execute(const ProgramNode& program) {
  if (use_bytecode_ && compiler_ && vm_) {
    // Use bytecode compilation and execution
    auto chunk = compiler_->compile(program);
    if (!chunk) {
      runtime_error("Failed to compile program to bytecode");
      return PEBBLObject::make_null();
    }
    
    // Transfer global variables from interpreter environment to VM
    sync_globals_to_vm();
    
    VMResult result = vm_->execute(*chunk);
    if (result != VMResult::OK) {
      runtime_error("VM execution failed: " + vm_->get_error());
      return PEBBLObject::make_null();
    }
    
    // Sync globals back from VM to interpreter
    sync_globals_from_vm();
    
    return vm_->get_result();
  } else {
    // Use tree-walking interpretation (original behavior)
    PEBBLObject result = PEBBLObject::make_null();

    // Reset return state for each program execution
    has_return_ = false;
    return_value_ = PEBBLObject::make_null();

    for (const auto& statement : program.statements) {
      // Ensure we're always in the global environment for top-level statements
      current_env_ = global_env_;
      result = execute(*statement);
      if (has_return_) {
        break;
      }
    }

    return result;
  }
}

PEBBLObject Interpreter::evaluate(const ExpressionNode& expr) {
  switch (expr.type()) {
    case ASTType::INTEGER_LITERAL:
    case ASTType::FLOAT_LITERAL:
    case ASTType::STRING_LITERAL:
    case ASTType::BOOLEAN_LITERAL:
      return evaluate_literal(static_cast<const LiteralNode&>(expr));

    case ASTType::ARRAY_LITERAL:
      return evaluate_array_literal(static_cast<const ArrayLiteralNode&>(expr));

    case ASTType::DICT_LITERAL:
      return evaluate_dict_literal(static_cast<const DictLiteralNode&>(expr));

    case ASTType::IDENTIFIER:
      return evaluate_identifier(static_cast<const IdentifierNode&>(expr));

    case ASTType::BINARY_EXPRESSION:
      return evaluate_binary(static_cast<const BinaryExpressionNode&>(expr));

    case ASTType::UNARY_EXPRESSION:
      return evaluate_unary(static_cast<const UnaryExpressionNode&>(expr));

    case ASTType::ASSIGNMENT_EXPRESSION:
      return evaluate_assignment(static_cast<const AssignmentExpressionNode&>(expr));

    case ASTType::IF_ELSE_EXPRESSION:
      return evaluate_if_else(static_cast<const IfElseExpressionNode&>(expr));

    case ASTType::CALL_EXPRESSION:
      return evaluate_call(static_cast<const CallExpressionNode&>(expr));

    default:
      runtime_error("Unknown expression type", expr.get_token());
      return PEBBLObject::make_null();
  }
}

PEBBLObject Interpreter::execute(const StatementNode& stmt) {
  switch (stmt.type()) {
    case ASTType::EXPRESSION_STATEMENT:
      return execute_expression_statement(static_cast<const ExpressionStatementNode&>(stmt));

    case ASTType::VARIABLE_STATEMENT:
      return execute_variable_statement(static_cast<const VariableStatementNode&>(stmt));

    case ASTType::RETURN_STATEMENT:
      return execute_return_statement(static_cast<const ReturnStatementNode&>(stmt));

    case ASTType::BLOCK_STATEMENT:
      return execute_block_statement(static_cast<const BlockStatementNode&>(stmt));

    case ASTType::WHILE_LOOP_STATEMENT:
      return execute_while_statement(static_cast<const WhileLoopStatementNode&>(stmt));

    case ASTType::FOR_LOOP_STATEMENT:
      return execute_for_statement(static_cast<const ForLoopStatementNode&>(stmt));

    case ASTType::FUNCTION_STATEMENT:
      return execute_function_statement(static_cast<const FunctionStatementNode&>(stmt));

    default:
      runtime_error("Unknown statement type", stmt.get_token());
      return PEBBLObject::make_null();
  }
}

PEBBLObject Interpreter::evaluate_literal(const LiteralNode& expr) {
  switch (expr.type()) {
    case ASTType::INTEGER_LITERAL: {
      const auto& int_literal = static_cast<const IntegerLiteralNode&>(expr);
      // For now, convert to int32 (could extend to handle larger integers)
      int32_t value = static_cast<int32_t>(int_literal.value);
      return PEBBLObject::make_int32(value);
    }

    case ASTType::FLOAT_LITERAL: {
      const auto& float_literal = static_cast<const FloatLiteralNode&>(expr);
      return PEBBLObject::make_double(float_literal.value);
    }

    case ASTType::STRING_LITERAL: {
      const auto& string_literal = static_cast<const StringLiteralNode&>(expr);
      auto* str_obj = heap_.allocate<PEBBLString>(string_literal.value);
      return PEBBLObject::make_gc_ptr(str_obj);
    }

    case ASTType::BOOLEAN_LITERAL: {
      const auto& bool_literal = static_cast<const BooleanLiteralNode&>(expr);
      return PEBBLObject::make_bool(bool_literal.value);
    }

    default:
      runtime_error("Invalid literal type", expr.get_token());
      return PEBBLObject::make_null();
  }
}

PEBBLObject Interpreter::evaluate_identifier(const IdentifierNode& expr) {
  try {
    return current_env_->get(expr.name);
  } catch (const std::runtime_error& e) {
    runtime_error(e.what(), expr.get_token());
    return PEBBLObject::make_null();
  }
}

PEBBLObject Interpreter::evaluate_binary(const BinaryExpressionNode& expr) {
  PEBBLObject left = evaluate(*expr.left);
  PEBBLObject right = evaluate(*expr.right);

  switch (expr.operator_token.type) {
    case TokenType::PLUS:
      if (left.is_int32() && right.is_int32()) {
        return PEBBLObject::make_int32(left.as_int32() + right.as_int32());
      } else if (left.is_double() && right.is_double()) {
        return PEBBLObject::make_double(left.as_double() + right.as_double());
      } else if (left.is_int32() && right.is_double()) {
        return PEBBLObject::make_double(left.as_int32() + right.as_double());
      } else if (left.is_double() && right.is_int32()) {
        return PEBBLObject::make_double(left.as_double() + right.as_int32());
      }
      runtime_error("Invalid operands for +", &expr.operator_token);
      break;

    case TokenType::MINUS:
      if (left.is_int32() && right.is_int32()) {
        return PEBBLObject::make_int32(left.as_int32() - right.as_int32());
      } else if (left.is_double() && right.is_double()) {
        return PEBBLObject::make_double(left.as_double() - right.as_double());
      } else if (left.is_int32() && right.is_double()) {
        return PEBBLObject::make_double(left.as_int32() - right.as_double());
      } else if (left.is_double() && right.is_int32()) {
        return PEBBLObject::make_double(left.as_double() - right.as_int32());
      }
      runtime_error("Invalid operands for -", &expr.operator_token);
      break;

    case TokenType::ASTERISK:
      if (left.is_int32() && right.is_int32()) {
        return PEBBLObject::make_int32(left.as_int32() * right.as_int32());
      } else if (left.is_double() && right.is_double()) {
        return PEBBLObject::make_double(left.as_double() * right.as_double());
      } else if (left.is_int32() && right.is_double()) {
        return PEBBLObject::make_double(left.as_int32() * right.as_double());
      } else if (left.is_double() && right.is_int32()) {
        return PEBBLObject::make_double(left.as_double() * right.as_int32());
      }
      runtime_error("Invalid operands for *", &expr.operator_token);
      break;

    case TokenType::SLASH:
      if (left.is_int32() && right.is_int32()) {
        if (right.as_int32() == 0) {
          runtime_error("Division by zero", &expr.operator_token);
        }
        return PEBBLObject::make_double(static_cast<double>(left.as_int32()) / right.as_int32());
      } else if (left.is_double() && right.is_double()) {
        if (right.as_double() == 0.0) {
          runtime_error("Division by zero", &expr.operator_token);
        }
        return PEBBLObject::make_double(left.as_double() / right.as_double());
      } else if (left.is_int32() && right.is_double()) {
        if (right.as_double() == 0.0) {
          runtime_error("Division by zero", &expr.operator_token);
        }
        return PEBBLObject::make_double(left.as_int32() / right.as_double());
      } else if (left.is_double() && right.is_int32()) {
        if (right.as_int32() == 0) {
          runtime_error("Division by zero", &expr.operator_token);
        }
        return PEBBLObject::make_double(left.as_double() / right.as_int32());
      }
      runtime_error("Invalid operands for /", &expr.operator_token);
      break;

    case TokenType::EQUAL:
      return PEBBLObject::make_bool(are_equal(left, right));

    case TokenType::NOT_EQUAL:
      return PEBBLObject::make_bool(!are_equal(left, right));

    case TokenType::LESS:
      if (left.is_int32() && right.is_int32()) {
        return PEBBLObject::make_bool(left.as_int32() < right.as_int32());
      } else if (left.is_double() && right.is_double()) {
        return PEBBLObject::make_bool(left.as_double() < right.as_double());
      } else if (left.is_int32() && right.is_double()) {
        return PEBBLObject::make_bool(left.as_int32() < right.as_double());
      } else if (left.is_double() && right.is_int32()) {
        return PEBBLObject::make_bool(left.as_double() < right.as_int32());
      }
      runtime_error("Invalid operands for <", &expr.operator_token);
      break;

    case TokenType::GREATER:
      if (left.is_int32() && right.is_int32()) {
        return PEBBLObject::make_bool(left.as_int32() > right.as_int32());
      } else if (left.is_double() && right.is_double()) {
        return PEBBLObject::make_bool(left.as_double() > right.as_double());
      } else if (left.is_int32() && right.is_double()) {
        return PEBBLObject::make_bool(left.as_int32() > right.as_double());
      } else if (left.is_double() && right.is_int32()) {
        return PEBBLObject::make_bool(left.as_double() > right.as_int32());
      }
      runtime_error("Invalid operands for >", &expr.operator_token);
      break;

    case TokenType::LESS_EQUAL:
      if (left.is_int32() && right.is_int32()) {
        return PEBBLObject::make_bool(left.as_int32() <= right.as_int32());
      } else if (left.is_double() && right.is_double()) {
        return PEBBLObject::make_bool(left.as_double() <= right.as_double());
      } else if (left.is_int32() && right.is_double()) {
        return PEBBLObject::make_bool(left.as_int32() <= right.as_double());
      } else if (left.is_double() && right.is_int32()) {
        return PEBBLObject::make_bool(left.as_double() <= right.as_int32());
      }
      runtime_error("Invalid operands for <=", &expr.operator_token);
      break;

    case TokenType::GREATER_EQUAL:
      if (left.is_int32() && right.is_int32()) {
        return PEBBLObject::make_bool(left.as_int32() >= right.as_int32());
      } else if (left.is_double() && right.is_double()) {
        return PEBBLObject::make_bool(left.as_double() >= right.as_double());
      } else if (left.is_int32() && right.is_double()) {
        return PEBBLObject::make_bool(left.as_int32() >= right.as_double());
      } else if (left.is_double() && right.is_int32()) {
        return PEBBLObject::make_bool(left.as_double() >= right.as_int32());
      }
      runtime_error("Invalid operands for >=", &expr.operator_token);
      break;

    case TokenType::AND:
      return PEBBLObject::make_bool(is_truthy(left) && is_truthy(right));

    case TokenType::OR:
      return PEBBLObject::make_bool(is_truthy(left) || is_truthy(right));

    default:
      runtime_error("Unknown binary operator", &expr.operator_token);
  }

  return PEBBLObject::make_null();
}

PEBBLObject Interpreter::evaluate_unary(const UnaryExpressionNode& expr) {
  PEBBLObject operand = evaluate(*expr.operand);

  switch (expr.operator_token.type) {
    case TokenType::MINUS:
      if (operand.is_int32()) {
        return PEBBLObject::make_int32(-operand.as_int32());
      } else if (operand.is_double()) {
        return PEBBLObject::make_double(-operand.as_double());
      }
      runtime_error("Invalid operand for unary -", &expr.operator_token);
      break;

    case TokenType::BANG:
      return PEBBLObject::make_bool(!is_truthy(operand));

    default:
      runtime_error("Unknown unary operator", &expr.operator_token);
  }

  return PEBBLObject::make_null();
}

PEBBLObject Interpreter::evaluate_assignment(const AssignmentExpressionNode& expr) {
  PEBBLObject value = evaluate(*expr.value);

  if (expr.target->type() == ASTType::IDENTIFIER) {
    const auto& identifier = static_cast<const IdentifierNode&>(*expr.target);
    try {
      current_env_->set(identifier.name, value);
      return value;
    } catch (const std::runtime_error& e) {
      runtime_error(e.what(), expr.get_token());
    }
  } else {
    runtime_error("Invalid assignment target", expr.get_token());
  }

  return PEBBLObject::make_null();
}

PEBBLObject Interpreter::evaluate_if_else(const IfElseExpressionNode& expr) {
  PEBBLObject condition = evaluate(*expr.condition);

  if (is_truthy(condition)) {
    return evaluate(*expr.then_expression);
  } else if (expr.else_expression) {
    return evaluate(*expr.else_expression);
  }

  return PEBBLObject::make_null();
}

PEBBLObject Interpreter::evaluate_array_literal(const ArrayLiteralNode& expr) {
  std::vector<PEBBLObject> elements;
  elements.reserve(expr.elements.size());

  for (const auto& element : expr.elements) {
    elements.push_back(evaluate(*element));
  }

  auto* array_obj = heap_.allocate<PEBBLArray>(std::move(elements));
  return PEBBLObject::make_gc_ptr(array_obj);
}

PEBBLObject Interpreter::evaluate_dict_literal(const DictLiteralNode& expr) {
  std::unordered_map<std::string, PEBBLObject> entries;

  for (const auto& [key_ptr, value_ptr] : expr.entries) {
    PEBBLObject key_value = evaluate(*key_ptr);
    PEBBLObject value = evaluate(*value_ptr);

    std::string key_str;
    if (key_value.is_gc_ptr()) {
      auto* gc_obj = key_value.as_gc_ptr();
      if (gc_obj->tag == GCTag::STRING) {
        auto* str_obj = static_cast<PEBBLString*>(gc_obj);
        key_str = str_obj->value;
      } else {
        runtime_error("Dictionary keys must be strings", expr.get_token());
        return PEBBLObject::make_null();
      }
    } else {
      runtime_error("Dictionary keys must be strings", expr.get_token());
      return PEBBLObject::make_null();
    }

    entries[key_str] = value;
  }

  auto* dict_obj = heap_.allocate<PEBBLDict>(std::move(entries));
  return PEBBLObject::make_gc_ptr(dict_obj);
}

bool Interpreter::is_truthy(PEBBLObject value) {
  if (value.is_bool()) {
    return value.as_bool();
  } else if (value.is_null()) {
    return false;
  } else if (value.is_int32()) {
    return value.as_int32() != 0;
  } else if (value.is_double()) {
    return value.as_double() != 0.0;
  }
  return true;
}

bool Interpreter::are_equal(PEBBLObject left, PEBBLObject right) {
  if (left.is_null() && right.is_null()) return true;
  if (left.is_null() || right.is_null()) return false;

  if (left.is_bool() && right.is_bool()) {
    return left.as_bool() == right.as_bool();
  }

  if (left.is_int32() && right.is_int32()) {
    return left.as_int32() == right.as_int32();
  }

  if (left.is_double() && right.is_double()) {
    return left.as_double() == right.as_double();
  }

  if ((left.is_int32() && right.is_double()) || (left.is_double() && right.is_int32())) {
    double left_val = left.is_int32() ? left.as_int32() : left.as_double();
    double right_val = right.is_int32() ? right.as_int32() : right.as_double();
    return left_val == right_val;
  }

  if (left.is_gc_ptr() && right.is_gc_ptr()) {
    return left.as_gc_ptr() == right.as_gc_ptr();
  }

  return false;
}

std::string Interpreter::stringify(PEBBLObject value) {
  if (value.is_null()) {
    return "nil";
  } else if (value.is_bool()) {
    return value.as_bool() ? "true" : "false";
  } else if (value.is_int32()) {
    return std::to_string(value.as_int32());
  } else if (value.is_double()) {
    return std::to_string(value.as_double());
  } else if (value.is_gc_ptr()) {
    auto* gc_obj = value.as_gc_ptr();
    switch (gc_obj->tag) {
      case GCTag::STRING:
        return static_cast<PEBBLString*>(gc_obj)->value;
      case GCTag::ARRAY: {
        auto* array = static_cast<PEBBLArray*>(gc_obj);
        std::stringstream ss;
        ss << "[";
        for (size_t i = 0; i < array->elements.size(); ++i) {
          if (i > 0) ss << ", ";
          ss << stringify(array->elements[i]);
        }
        ss << "]";
        return ss.str();
      }
      case GCTag::DICT: {
        auto* dict = static_cast<PEBBLDict*>(gc_obj);
        std::stringstream ss;
        ss << "{";
        bool first = true;
        for (const auto& [key, val] : dict->entries) {
          if (!first) ss << ", ";
          first = false;
          ss << "\"" << key << "\": " << stringify(val);
        }
        ss << "}";
        return ss.str();
      }
      case GCTag::FUNCTION: {
        auto* func = static_cast<PEBBLFunction*>(gc_obj);
        return "<function " + func->name + ">";
      }
      case GCTag::BUILTIN_FUNCTION: {
        auto* builtin = static_cast<PEBBLBuiltinFunction*>(gc_obj);
        return "<builtin " + builtin->name + ">";
      }
      default:
        return "<object>";
    }
  }
  return "<unknown>";
}

PEBBLObject Interpreter::execute_expression_statement(const ExpressionStatementNode& stmt) {
  return evaluate(*stmt.expression);
}

PEBBLObject Interpreter::execute_variable_statement(const VariableStatementNode& stmt) {
  PEBBLObject value = evaluate(*stmt.value);
  bool is_mutable = stmt.is_mutable();

  current_env_->define(stmt.name->name, value, is_mutable);
  return PEBBLObject::make_null();
}

PEBBLObject Interpreter::execute_return_statement(const ReturnStatementNode& stmt) {
  PEBBLObject value = PEBBLObject::make_null();
  if (stmt.return_value) {
    value = evaluate(*stmt.return_value);
  }

  has_return_ = true;
  return_value_ = value;
  return value;
}

PEBBLObject Interpreter::execute_block_statement(const BlockStatementNode& stmt) {
  auto block_env = std::make_shared<Environment>(current_env_);
  push_environment(block_env);

  PEBBLObject result = PEBBLObject::make_null();

  try {
    for (const auto& statement : stmt.statements) {
      result = execute(*statement);
      if (has_return_) {
        break;
      }
    }
  } catch (...) {
    pop_environment();
    throw;
  }

  pop_environment();
  return result;
}

PEBBLObject Interpreter::execute_while_statement(const WhileLoopStatementNode& stmt) {
  PEBBLObject result = PEBBLObject::make_null();

  while (is_truthy(evaluate(*stmt.condition))) {
    result = execute(*stmt.block);
    if (has_return_) {
      break;
    }
  }

  return result;
}

PEBBLObject Interpreter::execute_for_statement(const ForLoopStatementNode& stmt) {
  // Evaluate the iterable expression
  PEBBLObject iterable = evaluate(*stmt.iterable);

  if (iterable.is_null()) {
    runtime_error("Cannot iterate over null value", stmt.get_token());
    return PEBBLObject::make_null();
  }

  // Create new scope for the loop
  auto loop_env = std::make_shared<Environment>(current_env_);
  push_environment(loop_env);

  PEBBLObject result = PEBBLObject::make_null();

  try {
    if (iterable.is_gc_ptr()) {
      auto* gc_obj = iterable.as_gc_ptr();

      if (gc_obj->tag == GCTag::ARRAY) {
        // Iterate over array elements
        auto* array = static_cast<PEBBLArray*>(gc_obj);
        for (const auto& element : array->elements) {
          // Bind loop variable to current element
          // Use define for first iteration, then set for subsequent ones
          if (!current_env_->exists(stmt.identifier->name)) {
            current_env_->define(stmt.identifier->name, element, true);
          } else {
            current_env_->set(stmt.identifier->name, element);
          }

          // Execute loop body
          result = execute(*stmt.body);

          // Check for return statement
          if (has_return_) {
            break;
          }
        }
      } else if (gc_obj->tag == GCTag::DICT) {
        // Iterate over dictionary keys
        auto* dict = static_cast<PEBBLDict*>(gc_obj);
        for (const auto& [key, value] : dict->entries) {
          // Bind loop variable to current key
          PEBBLObject key_obj = PEBBLObject::make_gc_ptr(heap_.allocate<PEBBLString>(key));
          // Use define for first iteration, then set for subsequent ones
          if (!current_env_->exists(stmt.identifier->name)) {
            current_env_->define(stmt.identifier->name, key_obj, true);
          } else {
            current_env_->set(stmt.identifier->name, key_obj);
          }

          // Execute loop body
          result = execute(*stmt.body);

          // Check for return statement
          if (has_return_) {
            break;
          }
        }
      } else {
        runtime_error("Object is not iterable", stmt.get_token());
      }
    } else {
      runtime_error("Value is not iterable", stmt.get_token());
    }
  } catch (...) {
    // Restore previous environment on exception
    pop_environment();
    throw;
  }

  // Restore previous environment
  pop_environment();

  return result;
}

void Interpreter::push_environment(std::shared_ptr<Environment> env) {
  current_env_ = env;
}

void Interpreter::pop_environment() {
  if (current_env_->get_parent()) {
    current_env_ = current_env_->get_parent();
  }
}

PEBBLObject Interpreter::execute_function_statement(const FunctionStatementNode& stmt) {
  // Convert parameter nodes to strings
  std::vector<std::string> param_names;
  param_names.reserve(stmt.parameters.size());
  for (const auto& param : stmt.parameters) {
    param_names.push_back(param->name);
  }

  // Create function object with current environment as closure
  auto func = heap_.allocate<PEBBLFunction>(
      stmt.name->name, std::move(param_names), current_env_, stmt.body.get());

  // Define function in current environment
  PEBBLObject func_obj = PEBBLObject::make_gc_ptr(func);
  current_env_->define(stmt.name->name, func_obj, false);  // Functions are immutable by default

  return PEBBLObject::make_null();
}

PEBBLObject Interpreter::evaluate_call(const CallExpressionNode& expr) {
  // Evaluate the function expression
  PEBBLObject function = evaluate(*expr.function);

  if (!function.is_gc_ptr()) {
    runtime_error("Not a function", expr.get_token());
    return PEBBLObject::make_null();
  }

  auto* gc_obj = function.as_gc_ptr();

  if (gc_obj->tag == GCTag::BUILTIN_FUNCTION) {
    auto* builtin_func = static_cast<PEBBLBuiltinFunction*>(gc_obj);

    // Check arity for functions that have fixed arity (SIZE_MAX means variable arguments)
    if (builtin_func->arity != SIZE_MAX && expr.arguments.size() != builtin_func->arity) {
      runtime_error(
          "Wrong number of arguments. Expected " + std::to_string(builtin_func->arity) + ", got " +
              std::to_string(expr.arguments.size()),
          expr.get_token());
      return PEBBLObject::make_null();
    }

    // Evaluate arguments
    std::vector<PEBBLObject> args;
    for (const auto& arg : expr.arguments) {
      args.push_back(evaluate(*arg));
    }

    return builtin_func->function(args, *this);
  }

  if (gc_obj->tag != GCTag::FUNCTION) {
    runtime_error("Not a function", expr.get_token());
    return PEBBLObject::make_null();
  }

  auto* func = static_cast<PEBBLFunction*>(gc_obj);

  // Check arity
  if (expr.arguments.size() != func->arity()) {
    runtime_error(
        "Wrong number of arguments. Expected " + std::to_string(func->arity()) + ", got " +
            std::to_string(expr.arguments.size()),
        expr.get_token());
    return PEBBLObject::make_null();
  }

  // Evaluate arguments
  std::vector<PEBBLObject> arg_values;
  arg_values.reserve(expr.arguments.size());
  for (const auto& arg : expr.arguments) {
    arg_values.push_back(evaluate(*arg));
  }

  // Create new environment for function execution
  auto call_env = std::make_shared<Environment>(func->closure);

  // Bind parameters to arguments
  for (size_t i = 0; i < func->parameters.size(); ++i) {
    call_env->define(func->parameters[i], arg_values[i], true);
  }

  // Save current environment and switch to call environment
  auto prev_env = current_env_;
  auto prev_return = has_return_;
  auto prev_return_value = return_value_;

  current_env_ = call_env;
  has_return_ = false;
  return_value_ = PEBBLObject::make_null();

  PEBBLObject result = PEBBLObject::make_null();

  try {
    // Execute function body
    result = execute(*func->body);

    // If function explicitly returned, use that value
    if (has_return_) {
      result = return_value_;
    }
  } catch (...) {
    // Restore state on exception
    current_env_ = prev_env;
    has_return_ = prev_return;
    return_value_ = prev_return_value;
    throw;
  }

  // Restore previous state
  current_env_ = prev_env;
  has_return_ = prev_return;
  return_value_ = prev_return_value;

  return result;
}

void Interpreter::runtime_error(const std::string& message, const Token* token) {
  std::cerr << "Runtime Error";
  if (token) {
    std::cerr << " at line " << token->line;
  }
  std::cerr << ": " << message << std::endl;
  throw RuntimeError(message, token);
}

void Interpreter::register_builtin_functions() {
  // Register print function (variable arguments)
  auto* print_builtin =
      heap_.allocate<PEBBLBuiltinFunction>("print", SIZE_MAX, BuiltinFunctions::print_impl);
  global_env_->define("print", PEBBLObject::make_gc_ptr(print_builtin), false);

  // Register length function
  auto* length_builtin =
      heap_.allocate<PEBBLBuiltinFunction>("length", 1, BuiltinFunctions::length_impl);
  global_env_->define("length", PEBBLObject::make_gc_ptr(length_builtin), false);

  // Register type function
  auto* type_builtin = heap_.allocate<PEBBLBuiltinFunction>("type", 1, BuiltinFunctions::type_impl);
  global_env_->define("type", PEBBLObject::make_gc_ptr(type_builtin), false);

  // Register str function
  auto* str_builtin = heap_.allocate<PEBBLBuiltinFunction>("str", 1, BuiltinFunctions::str_impl);
  global_env_->define("str", PEBBLObject::make_gc_ptr(str_builtin), false);

  // Register push function
  auto* push_builtin = heap_.allocate<PEBBLBuiltinFunction>("push", 2, BuiltinFunctions::push_impl);
  global_env_->define("push", PEBBLObject::make_gc_ptr(push_builtin), false);

  // Register pop function
  auto* pop_builtin = heap_.allocate<PEBBLBuiltinFunction>("pop", 1, BuiltinFunctions::pop_impl);
  global_env_->define("pop", PEBBLObject::make_gc_ptr(pop_builtin), false);
}

void Interpreter::trace_roots(Tracer& tracer) {
  // Trace all objects stored in the global environment
  trace_environment_objects(global_env_, tracer);

  // Also trace the current environment if it's different from global
  if (current_env_ != global_env_) {
    trace_environment_objects(current_env_, tracer);
  }

  // Trace the return value if it's a GC object
  if (return_value_.is_gc_ptr()) {
    tracer.mark(return_value_.as_gc_ptr());
  }
}

void Interpreter::trace_environment_objects(std::shared_ptr<Environment> env, Tracer& tracer) {
  if (!env) return;

  // Trace all objects in this environment
  env->trace_objects(tracer);

  // Recursively trace parent environments
  trace_environment_objects(env->get_parent(), tracer);
}

void Interpreter::set_bytecode_mode(bool enable) {
  use_bytecode_ = enable;
  
  if (enable && !compiler_) {
    compiler_ = std::make_unique<Compiler>(heap_);
  }
  
  if (enable && !vm_) {
    vm_ = std::make_unique<VM>(heap_);
  }
}

void Interpreter::sync_globals_to_vm() {
  if (!vm_ || !global_env_) return;
  
  // For now, this is a simplified sync that only handles builtin functions
  // A full implementation would need to iterate through all global variables
  // and transfer them to the VM's global environment
  
  // TODO: Implement proper global variable synchronization
  // This would require access to Environment's internal storage
}

void Interpreter::sync_globals_from_vm() {
  if (!vm_ || !global_env_) return;
  
  // For now, this is a simplified sync
  // A full implementation would need to sync variables modified by the VM
  // back to the interpreter's environment
  
  // TODO: Implement proper global variable synchronization
}