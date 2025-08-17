/**
 * @file interpreter.cpp
 * @brief Implementation of the main interpreter
 */

#include "interpreter.hpp"
#include "builtin_objects.hpp"
#include <sstream>
#include <iostream>

Interpreter::Interpreter(GCHeap& heap) : heap_(heap) {
  global_env_ = std::make_shared<Environment>();
  current_env_ = global_env_;
}

PEBBLObject Interpreter::execute(const ProgramNode& program) {
  PEBBLObject result = PEBBLObject::make_null();
  
  for (const auto& statement : program.statements) {
    result = execute(*statement);
    if (has_return_) {
      break;
    }
  }
  
  return result;
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
  bool is_mutable = stmt.is_mutible();
  
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
          current_env_->define(stmt.identifier->name, element, true);
          
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
          current_env_->define(stmt.identifier->name, key_obj, true);
          
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

void Interpreter::runtime_error(const std::string& message, const Token* token) {
  std::cerr << "Runtime Error";
  if (token) {
    std::cerr << " at line " << token->line;
  }
  std::cerr << ": " << message << std::endl;
  throw RuntimeError(message, token);
}