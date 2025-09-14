/**
 * @file vm.cpp
 * @brief Implementation of the PEBBL virtual machine
 */

#include "vm.hpp"
#include "builtin_funcs.hpp"
#include "builtin_objects.hpp"
#include <iostream>
#include <sstream>


VM::VM(GCHeap& heap) : heap_(heap), has_error_(false) {
  stack_.reserve(STACK_MAX);
  frames_.reserve(FRAMES_MAX);
  
  global_env_ = std::make_shared<Environment>();
  current_env_ = global_env_;
  
  // Register this VM as a GC root tracer
  heap_.add_root_tracer([this](Tracer& tracer) { this->trace_roots(tracer); });
  
  // For now, skip builtin registration - will be handled during integration
}

VMResult VM::execute(const Chunk& chunk) {
  reset();
  
  // Push initial call frame
  frames_.emplace_back(&chunk, 0, 0);
  
  return run();
}

PEBBLObject VM::get_result() const {
  if (stack_.empty()) {
    return PEBBLObject::make_null();
  }
  return stack_.back();
}

void VM::reset() {
  stack_.clear();
  frames_.clear();
  has_error_ = false;
  error_message_.clear();
  current_env_ = global_env_;
}

VMResult VM::run() {
  while (!frames_.empty()) {
    CallFrame& frame = current_frame();
    const Chunk& chunk = *frame.chunk;
    
    if (frame.instruction_pointer >= chunk.instructions.size()) {
      // End of chunk reached
      if (frames_.size() == 1) {
        // Main program finished
        break;
      } else {
        // Function finished, pop frame
        frames_.pop_back();
        continue;
      }
    }
    
    const Instruction& instruction = chunk.instructions[frame.instruction_pointer++];
    
    switch (instruction.opcode) {
      case OpCode::LOAD_CONST:
        handle_load_const(instruction.operand);
        break;
      case OpCode::LOAD_NULL:
        handle_load_null();
        break;
      case OpCode::LOAD_TRUE:
        handle_load_true();
        break;
      case OpCode::LOAD_FALSE:
        handle_load_false();
        break;
      case OpCode::LOAD_VAR:
        handle_load_var(instruction.operand);
        break;
      case OpCode::STORE_VAR:
        handle_store_var(instruction.operand);
        break;
      case OpCode::DEFINE_VAR:
        handle_define_var(instruction.operand);
        break;
      case OpCode::ADD:
        handle_add();
        break;
      case OpCode::SUBTRACT:
        handle_subtract();
        break;
      case OpCode::MULTIPLY:
        handle_multiply();
        break;
      case OpCode::DIVIDE:
        handle_divide();
        break;
      case OpCode::NEGATE:
        handle_negate();
        break;
      case OpCode::EQUAL:
        handle_equal();
        break;
      case OpCode::NOT_EQUAL:
        handle_not_equal();
        break;
      case OpCode::LESS:
        handle_less();
        break;
      case OpCode::GREATER:
        handle_greater();
        break;
      case OpCode::LESS_EQUAL:
        handle_less_equal();
        break;
      case OpCode::GREATER_EQUAL:
        handle_greater_equal();
        break;
      case OpCode::NOT:
        handle_not();
        break;
      case OpCode::AND:
        handle_and();
        break;
      case OpCode::OR:
        handle_or();
        break;
      case OpCode::JUMP:
        handle_jump(instruction.operand);
        break;
      case OpCode::JUMP_IF_FALSE:
        handle_jump_if_false(instruction.operand);
        break;
      case OpCode::JUMP_IF_TRUE:
        handle_jump_if_true(instruction.operand);
        break;
      case OpCode::CALL:
        handle_call(instruction.operand);
        break;
      case OpCode::RETURN:
        handle_return();
        break;
      case OpCode::BUILD_ARRAY:
        handle_build_array(instruction.operand);
        break;
      case OpCode::BUILD_DICT:
        handle_build_dict(instruction.operand);
        break;
      case OpCode::POP:
        handle_pop();
        break;
      case OpCode::DUP:
        handle_dup();
        break;
      case OpCode::HALT:
        return VMResult::OK;
      default:
        runtime_error("Unknown instruction: " + std::to_string(static_cast<int>(instruction.opcode)));
        return VMResult::RUNTIME_ERROR;
    }
    
    if (has_error_) {
      return VMResult::RUNTIME_ERROR;
    }
  }
  
  return VMResult::OK;
}

void VM::push(PEBBLObject value) {
  if (stack_.size() >= STACK_MAX) {
    runtime_error("Stack overflow");
    return;
  }
  stack_.push_back(value);
}

PEBBLObject VM::pop() {
  if (stack_.empty()) {
    runtime_error("Stack underflow");
    return PEBBLObject::make_null();
  }
  PEBBLObject value = stack_.back();
  stack_.pop_back();
  return value;
}

PEBBLObject VM::peek(uint32_t distance) {
  if (distance >= stack_.size()) {
    runtime_error("Stack underflow in peek");
    return PEBBLObject::make_null();
  }
  return stack_[stack_.size() - 1 - distance];
}

void VM::reset_stack() {
  stack_.clear();
}

void VM::handle_load_const(uint32_t operand) {
  const Chunk& chunk = current_chunk();
  if (operand >= chunk.constants.size()) {
    runtime_error("Invalid constant index: " + std::to_string(operand));
    return;
  }
  push(chunk.constants[operand]);
}

void VM::handle_load_null() {
  push(PEBBLObject::make_null());
}

void VM::handle_load_true() {
  push(PEBBLObject::make_bool(true));
}

void VM::handle_load_false() {
  push(PEBBLObject::make_bool(false));
}

void VM::handle_load_var(uint32_t operand) {
  const Chunk& chunk = current_chunk();
  if (operand >= chunk.variable_names.size()) {
    runtime_error("Invalid variable index: " + std::to_string(operand));
    return;
  }
  
  const std::string& var_name = chunk.variable_names[operand];
  try {
    PEBBLObject value = current_env_->get(var_name);
    push(value);
  } catch (const std::runtime_error& e) {
    runtime_error("Undefined variable '" + var_name + "'");
  }
}

void VM::handle_store_var(uint32_t operand) {
  const Chunk& chunk = current_chunk();
  if (operand >= chunk.variable_names.size()) {
    runtime_error("Invalid variable index: " + std::to_string(operand));
    return;
  }
  
  const std::string& var_name = chunk.variable_names[operand];
  PEBBLObject value = peek(0);  // Don't pop yet, assignment returns the value
  
  try {
    current_env_->set(var_name, value);
  } catch (const std::runtime_error& e) {
    runtime_error("Cannot assign to variable '" + var_name + "': " + e.what());
  }
}

void VM::handle_define_var(uint32_t operand) {
  const Chunk& chunk = current_chunk();
  if (operand >= chunk.variable_names.size()) {
    runtime_error("Invalid variable index: " + std::to_string(operand));
    return;
  }
  
  const std::string& var_name = chunk.variable_names[operand];
  PEBBLObject value = pop();
  
  // For simplicity, assume all variables are mutable
  current_env_->define(var_name, value, true);
}

void VM::handle_add() {
  PEBBLObject right = pop();
  PEBBLObject left = pop();
  PEBBLObject result;
  
  if (perform_numeric_operation(left, right, OpCode::ADD, result)) {
    push(result);
  } else {
    runtime_error("Invalid operands for addition");
  }
}

void VM::handle_subtract() {
  PEBBLObject right = pop();
  PEBBLObject left = pop();
  PEBBLObject result;
  
  if (perform_numeric_operation(left, right, OpCode::SUBTRACT, result)) {
    push(result);
  } else {
    runtime_error("Invalid operands for subtraction");
  }
}

void VM::handle_multiply() {
  PEBBLObject right = pop();
  PEBBLObject left = pop();
  PEBBLObject result;
  
  if (perform_numeric_operation(left, right, OpCode::MULTIPLY, result)) {
    push(result);
  } else {
    runtime_error("Invalid operands for multiplication");
  }
}

void VM::handle_divide() {
  PEBBLObject right = pop();
  PEBBLObject left = pop();
  PEBBLObject result;
  
  // Check for division by zero
  if ((right.is_int32() && right.as_int32() == 0) ||
      (right.is_double() && right.as_double() == 0.0)) {
    runtime_error("Division by zero");
    return;
  }
  
  if (perform_numeric_operation(left, right, OpCode::DIVIDE, result)) {
    push(result);
  } else {
    runtime_error("Invalid operands for division");
  }
}

void VM::handle_negate() {
  PEBBLObject operand = pop();
  
  if (operand.is_int32()) {
    push(PEBBLObject::make_int32(-operand.as_int32()));
  } else if (operand.is_double()) {
    push(PEBBLObject::make_double(-operand.as_double()));
  } else {
    runtime_error("Invalid operand for negation");
  }
}

void VM::handle_equal() {
  PEBBLObject right = pop();
  PEBBLObject left = pop();
  push(PEBBLObject::make_bool(are_equal(left, right)));
}

void VM::handle_not_equal() {
  PEBBLObject right = pop();
  PEBBLObject left = pop();
  push(PEBBLObject::make_bool(!are_equal(left, right)));
}

void VM::handle_less() {
  PEBBLObject right = pop();
  PEBBLObject left = pop();
  PEBBLObject result;
  
  if (perform_comparison_operation(left, right, OpCode::LESS, result)) {
    push(result);
  } else {
    runtime_error("Invalid operands for less than comparison");
  }
}

void VM::handle_greater() {
  PEBBLObject right = pop();
  PEBBLObject left = pop();
  PEBBLObject result;
  
  if (perform_comparison_operation(left, right, OpCode::GREATER, result)) {
    push(result);
  } else {
    runtime_error("Invalid operands for greater than comparison");
  }
}

void VM::handle_less_equal() {
  PEBBLObject right = pop();
  PEBBLObject left = pop();
  PEBBLObject result;
  
  if (perform_comparison_operation(left, right, OpCode::LESS_EQUAL, result)) {
    push(result);
  } else {
    runtime_error("Invalid operands for less than or equal comparison");
  }
}

void VM::handle_greater_equal() {
  PEBBLObject right = pop();
  PEBBLObject left = pop();
  PEBBLObject result;
  
  if (perform_comparison_operation(left, right, OpCode::GREATER_EQUAL, result)) {
    push(result);
  } else {
    runtime_error("Invalid operands for greater than or equal comparison");
  }
}

void VM::handle_not() {
  PEBBLObject operand = pop();
  push(PEBBLObject::make_bool(!is_truthy(operand)));
}

void VM::handle_and() {
  PEBBLObject right = pop();
  PEBBLObject left = pop();
  push(PEBBLObject::make_bool(is_truthy(left) && is_truthy(right)));
}

void VM::handle_or() {
  PEBBLObject right = pop();
  PEBBLObject left = pop();
  push(PEBBLObject::make_bool(is_truthy(left) || is_truthy(right)));
}

void VM::handle_jump(uint32_t operand) {
  current_frame().instruction_pointer = operand;
}

void VM::handle_jump_if_false(uint32_t operand) {
  PEBBLObject condition = pop();
  if (!is_truthy(condition)) {
    current_frame().instruction_pointer = operand;
  }
}

void VM::handle_jump_if_true(uint32_t operand) {
  PEBBLObject condition = pop();
  if (is_truthy(condition)) {
    current_frame().instruction_pointer = operand;
  }
}

void VM::handle_call(uint32_t argc) {
  PEBBLObject function = peek(argc);  // Function is below the arguments
  
  if (!function.is_gc_ptr()) {
    runtime_error("Not a function");
    return;
  }
  
  auto* gc_obj = function.as_gc_ptr();
  
  if (gc_obj->tag == GCTag::BUILTIN_FUNCTION) {
    call_builtin(static_cast<PEBBLBuiltinFunction*>(gc_obj), argc);
  } else if (gc_obj->tag == GCTag::FUNCTION) {
    call_function(static_cast<PEBBLFunction*>(gc_obj), argc);
  } else {
    runtime_error("Not a callable object");
  }
}

void VM::handle_return() {
  PEBBLObject result = pop();
  
  if (frames_.size() <= 1) {
    // Returning from main program
    push(result);
    return;
  }
  
  // Pop the call frame
  CallFrame frame = frames_.back();
  frames_.pop_back();
  
  // Remove local variables from stack
  while (stack_.size() > frame.stack_base) {
    stack_.pop_back();
  }
  
  // Push return value
  push(result);
}

void VM::handle_build_array(uint32_t count) {
  std::vector<PEBBLObject> elements;
  elements.reserve(count);
  
  // Pop elements in reverse order
  for (uint32_t i = 0; i < count; ++i) {
    elements.insert(elements.begin(), pop());
  }
  
  auto* array_obj = heap_.allocate<PEBBLArray>(std::move(elements));
  push(PEBBLObject::make_gc_ptr(array_obj));
}

void VM::handle_build_dict(uint32_t count) {
  std::unordered_map<std::string, PEBBLObject> entries;
  
  // Pop key-value pairs
  for (uint32_t i = 0; i < count; ++i) {
    PEBBLObject value = pop();
    PEBBLObject key = pop();
    
    // Convert key to string
    if (key.is_gc_ptr() && key.as_gc_ptr()->tag == GCTag::STRING) {
      auto* str_obj = static_cast<PEBBLString*>(key.as_gc_ptr());
      entries[str_obj->value] = value;
    } else {
      runtime_error("Dictionary keys must be strings");
      return;
    }
  }
  
  auto* dict_obj = heap_.allocate<PEBBLDict>(std::move(entries));
  push(PEBBLObject::make_gc_ptr(dict_obj));
}

void VM::handle_pop() {
  pop();
}

void VM::handle_dup() {
  push(peek(0));
}

bool VM::is_truthy(PEBBLObject value) {
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

bool VM::are_equal(PEBBLObject left, PEBBLObject right) {
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

CallFrame& VM::current_frame() {
  return frames_.back();
}

const Chunk& VM::current_chunk() {
  return *current_frame().chunk;
}

void VM::runtime_error(const std::string& message) {
  has_error_ = true;
  error_message_ = message;
  std::cerr << "Runtime Error: " << message << std::endl;
}

void VM::runtime_error(const std::string& message, uint32_t instruction) {
  has_error_ = true;
  error_message_ = message;
  std::cerr << "Runtime Error at instruction " << instruction << ": " << message << std::endl;
}

bool VM::perform_numeric_operation(PEBBLObject left, PEBBLObject right, OpCode operation, PEBBLObject& result) {
  if (left.is_int32() && right.is_int32()) {
    int32_t a = left.as_int32();
    int32_t b = right.as_int32();
    switch (operation) {
      case OpCode::ADD:
        result = PEBBLObject::make_int32(a + b);
        return true;
      case OpCode::SUBTRACT:
        result = PEBBLObject::make_int32(a - b);
        return true;
      case OpCode::MULTIPLY:
        result = PEBBLObject::make_int32(a * b);
        return true;
      case OpCode::DIVIDE:
        result = PEBBLObject::make_double(static_cast<double>(a) / b);
        return true;
      default:
        return false;
    }
  } else if (left.is_double() || right.is_double()) {
    double a = left.is_int32() ? left.as_int32() : left.as_double();
    double b = right.is_int32() ? right.as_int32() : right.as_double();
    switch (operation) {
      case OpCode::ADD:
        result = PEBBLObject::make_double(a + b);
        return true;
      case OpCode::SUBTRACT:
        result = PEBBLObject::make_double(a - b);
        return true;
      case OpCode::MULTIPLY:
        result = PEBBLObject::make_double(a * b);
        return true;
      case OpCode::DIVIDE:
        result = PEBBLObject::make_double(a / b);
        return true;
      default:
        return false;
    }
  }
  return false;
}

bool VM::perform_comparison_operation(PEBBLObject left, PEBBLObject right, OpCode operation, PEBBLObject& result) {
  if (left.is_int32() && right.is_int32()) {
    int32_t a = left.as_int32();
    int32_t b = right.as_int32();
    switch (operation) {
      case OpCode::LESS:
        result = PEBBLObject::make_bool(a < b);
        return true;
      case OpCode::GREATER:
        result = PEBBLObject::make_bool(a > b);
        return true;
      case OpCode::LESS_EQUAL:
        result = PEBBLObject::make_bool(a <= b);
        return true;
      case OpCode::GREATER_EQUAL:
        result = PEBBLObject::make_bool(a >= b);
        return true;
      default:
        return false;
    }
  } else if (left.is_double() || right.is_double()) {
    double a = left.is_int32() ? left.as_int32() : left.as_double();
    double b = right.is_int32() ? right.as_int32() : right.as_double();
    switch (operation) {
      case OpCode::LESS:
        result = PEBBLObject::make_bool(a < b);
        return true;
      case OpCode::GREATER:
        result = PEBBLObject::make_bool(a > b);
        return true;
      case OpCode::LESS_EQUAL:
        result = PEBBLObject::make_bool(a <= b);
        return true;
      case OpCode::GREATER_EQUAL:
        result = PEBBLObject::make_bool(a >= b);
        return true;
      default:
        return false;
    }
  }
  return false;
}

bool VM::call_function(PEBBLFunction* function, uint32_t argc) {
  if (argc != function->arity()) {
    runtime_error("Wrong number of arguments. Expected " + 
                  std::to_string(function->arity()) + ", got " + 
                  std::to_string(argc));
    return false;
  }
  
  // TODO: Implement user-defined function calls
  // This would require:
  // 1. Creating a new call frame
  // 2. Setting up parameter bindings
  // 3. Executing the function's bytecode
  runtime_error("User-defined functions not yet implemented in VM");
  return false;
}

bool VM::call_builtin(PEBBLBuiltinFunction* function, uint32_t argc) {
  if (function->arity != SIZE_MAX && argc != function->arity) {
    runtime_error("Wrong number of arguments. Expected " + 
                  std::to_string(function->arity) + ", got " + 
                  std::to_string(argc));
    return false;
  }
  
  // Collect arguments
  std::vector<PEBBLObject> args;
  args.reserve(argc);
  for (uint32_t i = 0; i < argc; ++i) {
    args.insert(args.begin(), pop());
  }
  
  // Pop the function from the stack
  pop();
  
  // For now, we'll need to skip builtin function calls until we properly integrate
  // with the interpreter interface. This is a temporary limitation.
  runtime_error("Builtin function calls not yet implemented in VM: " + function->name);
  PEBBLObject result = PEBBLObject::make_null();
  push(result);
  
  return !has_error_;
}

void VM::set_global(const std::string& name, PEBBLObject value) {
  global_env_->define(name, value, false);
}

PEBBLObject VM::get_global(const std::string& name) {
  try {
    return global_env_->get(name);
  } catch (const std::runtime_error&) {
    return PEBBLObject::make_null();
  }
}

void VM::trace_roots(Tracer& tracer) {
  // Trace all objects on the stack
  for (const auto& value : stack_) {
    if (value.is_gc_ptr()) {
      tracer.mark(value.as_gc_ptr());
    }
  }
  
  // Trace all objects in the global environment
  if (global_env_) {
    global_env_->trace_objects(tracer);
  }
}

std::string VM::stringify(PEBBLObject value) {
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


