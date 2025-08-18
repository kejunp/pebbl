/**
 * @file object.hpp
 * @brief Runtime object system using NaN-boxing for efficient value representation
 */

#pragma once

#include <cstdint>

struct GCObject;

/**
 * @brief A value type that uses NaN-boxing to store different types efficiently
 *
 * PEBBLObject uses IEEE 754 double precision floating point NaN-boxing to store
 * different value types in a single 64-bit word. Regular doubles are stored directly,
 * while other types use the quiet NaN space with type tags.
 *
 * The bit layout for boxed values is:
 * - Bits 63-52: Exponent (all 1s for NaN)
 * - Bit 51: Quiet NaN bit (always 1)
 * - Bits 50-48: Type tag
 * - Bits 47-0: Payload data
 */
struct PEBBLObject {
  uint64_t bits;  ///< The raw 64-bit representation

  /// IEEE 754 exponent mask (bits 63-52)
  static constexpr uint64_t EXP_MASK = 0x7FF0'0000'0000'0000ULL;
  /// Quiet NaN mask (bit 51)
  static constexpr uint64_t QNAN_MASK = 0x0008'0000'0000'0000ULL;
  /// Base pattern for boxed values (EXP_MASK | QNAN_MASK)
  static constexpr uint64_t BOXED_BASE = EXP_MASK | QNAN_MASK;
  /// Mask for extracting type tag (bits 50-48)
  static constexpr uint64_t TAG_MASK = 0x0007'0000'0000'0000ULL;
  /// Bit shift amount for type tag
  static constexpr int TAG_SHIFT = 48;
  /// Mask for extracting payload data (bits 47-0)
  static constexpr uint64_t PAYLOAD_MASK = 0x0000'FFFF'FFFF'FFFFULL;

  /**
   * @brief Type tags for boxed values
   */
  enum class Tag : uint8_t {
    GC_PTR = 1,  ///< Garbage-collected pointer
    INT32,       ///< 32-bit signed integer
    BOOL,        ///< Boolean value
    NIL,         ///< Null/nil value
    UNDEFINED    ///< Undefined value
  };

  /**
   * @brief Default constructor creates a double value of 0.0
   */
  PEBBLObject() : bits(0) {
  }

  /**
   * @brief Create a PEBBLObject containing a double value
   * @param value The double value to store
   * @return A PEBBLObject containing the double value
   */
  static PEBBLObject make_double(double value);

  /**
   * @brief Create a PEBBLObject containing a 32-bit signed integer
   * @param value The integer value to store
   * @return A PEBBLObject containing the integer value
   */
  static PEBBLObject make_int32(int32_t value);

  /**
   * @brief Create a PEBBLObject containing a boolean value
   * @param value The boolean value to store
   * @return A PEBBLObject containing the boolean value
   */
  static PEBBLObject make_bool(bool value);

  /**
   * @brief Create a PEBBLObject containing a null/nil value
   * @return A PEBBLObject containing null
   */
  static PEBBLObject make_null();

  /**
   * @brief Create a PEBBLObject containing an undefined value
   * @return A PEBBLObject containing undefined
   */
  static PEBBLObject make_undefined();

  /**
   * @brief Create a PEBBLObject containing a garbage-collected pointer
   * @param ptr Pointer to a GCObject
   * @return A PEBBLObject containing the GC pointer
   */
  static PEBBLObject make_gc_ptr(GCObject* ptr);

  /**
   * @brief Check if this object contains a double value
   * @return true if the object contains a double, false otherwise
   */
  bool is_double() const;

  /**
   * @brief Check if this object is using the boxed representation
   * @return true if the object is boxed, false if it's a regular double
   */
  bool is_boxed() const;

  /**
   * @brief Get the type tag for boxed values
   * @return The type tag (only valid if is_boxed() returns true)
   */
  Tag get_tag() const;

  /**
   * @brief Check if this object contains a 32-bit integer
   * @return true if the object contains an int32, false otherwise
   */
  bool is_int32() const;

  /**
   * @brief Check if this object contains a boolean value
   * @return true if the object contains a boolean, false otherwise
   */
  bool is_bool() const;

  /**
   * @brief Check if this object contains a null/nil value
   * @return true if the object contains null, false otherwise
   */
  bool is_null() const;

  /**
   * @brief Check if this object contains an undefined value
   * @return true if the object contains undefined, false otherwise
   */
  bool is_undefined() const;

  /**
   * @brief Check if this object contains a garbage-collected pointer
   * @return true if the object contains a GC pointer, false otherwise
   */
  bool is_gc_ptr() const;

  /**
   * @brief Extract the double value from this object
   * @return The double value (undefined behavior if not a double)
   */
  double as_double() const;

  /**
   * @brief Extract the 32-bit integer value from this object
   * @return The integer value (undefined behavior if not an int32)
   */
  int32_t as_int32() const;

  /**
   * @brief Extract the boolean value from this object
   * @return The boolean value (undefined behavior if not a boolean)
   */
  bool as_bool() const;

  /**
   * @brief Extract the garbage-collected pointer from this object
   * @return The GC pointer (undefined behavior if not a GC pointer)
   */
  GCObject* as_gc_ptr() const;
};