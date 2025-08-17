#include "object.hpp"
#include "gc.hpp"
#include <cstring>

PEBBLObject PEBBLObject::make_double(double value) {
  PEBBLObject obj;
  memcpy(&obj.bits, &value, sizeof(double));
  return obj;
}

PEBBLObject PEBBLObject::make_int32(int32_t value) {
  PEBBLObject obj;
  obj.bits = BOXED_BASE | (static_cast<uint64_t>(Tag::INT32) << TAG_SHIFT) | 
             (static_cast<uint64_t>(value) & PAYLOAD_MASK);
  return obj;
}

PEBBLObject PEBBLObject::make_bool(bool value) {
  PEBBLObject obj;
  obj.bits = BOXED_BASE | (static_cast<uint64_t>(Tag::BOOL) << TAG_SHIFT) | 
             (value ? 1ULL : 0ULL);
  return obj;
}

PEBBLObject PEBBLObject::make_null() {
  PEBBLObject obj;
  obj.bits = BOXED_BASE | (static_cast<uint64_t>(Tag::NIL) << TAG_SHIFT);
  return obj;
}

PEBBLObject PEBBLObject::make_undefined() {
  PEBBLObject obj;
  obj.bits = BOXED_BASE | (static_cast<uint64_t>(Tag::UNDEFINED) << TAG_SHIFT);
  return obj;
}

PEBBLObject PEBBLObject::make_gc_ptr(GCObject* ptr) {
  PEBBLObject obj;
  obj.bits = BOXED_BASE | (static_cast<uint64_t>(Tag::GC_PTR) << TAG_SHIFT) | 
             (reinterpret_cast<uintptr_t>(ptr) & PAYLOAD_MASK);
  return obj;
}

bool PEBBLObject::is_double() const {
  return (bits & EXP_MASK) != EXP_MASK;
}

bool PEBBLObject::is_boxed() const {
  return (bits & BOXED_BASE) == BOXED_BASE;
}

PEBBLObject::Tag PEBBLObject::get_tag() const {
  return static_cast<Tag>((bits & TAG_MASK) >> TAG_SHIFT);
}

bool PEBBLObject::is_int32() const {
  return is_boxed() && get_tag() == Tag::INT32;
}

bool PEBBLObject::is_bool() const {
  return is_boxed() && get_tag() == Tag::BOOL;
}

bool PEBBLObject::is_null() const {
  return is_boxed() && get_tag() == Tag::NIL;
}

bool PEBBLObject::is_undefined() const {
  return is_boxed() && get_tag() == Tag::UNDEFINED;
}

bool PEBBLObject::is_gc_ptr() const {
  return is_boxed() && get_tag() == Tag::GC_PTR;
}

double PEBBLObject::as_double() const {
  double result;
  memcpy(&result, &bits, sizeof(double));
  return result;
}

int32_t PEBBLObject::as_int32() const {
  return static_cast<int32_t>(bits & PAYLOAD_MASK);
}

bool PEBBLObject::as_bool() const {
  return (bits & PAYLOAD_MASK) != 0;
}

GCObject* PEBBLObject::as_gc_ptr() const {
  return reinterpret_cast<GCObject*>(bits & PAYLOAD_MASK);
}