#pragma once

#include <cstdint>

struct GCObject;

struct PEBBLObject {
  uint64_t bits;

  static constexpr uint64_t EXP_MASK     = 0x7FF0'0000'0000'0000ULL;
  static constexpr uint64_t QNAN_MASK    = 0x0008'0000'0000'0000ULL;
  static constexpr uint64_t BOXED_BASE   = EXP_MASK | QNAN_MASK;
  static constexpr uint64_t TAG_MASK     = 0x0007'0000'0000'0000ULL;
  static constexpr int      TAG_SHIFT    = 48;
  static constexpr uint64_t PAYLOAD_MASK = 0x0000'FFFF'FFFF'FFFFULL;

  enum class Tag : uint8_t {GC_PTR = 1, INT32, BOOL, NIL, UNDEFINED};

  PEBBLObject() : bits(0) {}

  static PEBBLObject make_double(double value);
  static PEBBLObject make_int32(int32_t value);
  static PEBBLObject make_bool(bool value);
  static PEBBLObject make_null();
  static PEBBLObject make_undefined();
  static PEBBLObject make_gc_ptr(GCObject* ptr);

  bool is_double() const;
  bool is_boxed() const;
  Tag get_tag() const;
  bool is_int32() const;
  bool is_bool() const;
  bool is_null() const;
  bool is_undefined() const;
  bool is_gc_ptr() const;

  double as_double() const;
  int32_t as_int32() const;
  bool as_bool() const;
  GCObject* as_gc_ptr() const;
};