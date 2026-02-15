#ifndef ACRIS_LIB_STDTYPES_HPP
#define ACRIS_LIB_STDTYPES_HPP

// STL Includes:
#include <cstddef>
#include <cstdint>
// #include <stdfloat>

/*!
 * @file
 *
 * No namespace for these required, we use these as a default.
 */

// Aliases:
using uchar = unsigned char;

// Integer type aliases:
using uint = unsigned int;
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using isz = signed long;

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using usz = std::size_t;

// Float type aliases:
// TODO: Add f32 and f64 when clang libc++ supports them.
// TODO: As well as the Cereal serilization library.
// using f32 = std::float32_t;
// using f64 = std::float64_t;

using f32 = float;
using f64 = double;

// Make sure the fixed widths are enforced.
static_assert(sizeof(f32) == 4, "f32 is not 32-bit on this platform!");
static_assert(sizeof(f64) == 8, "f64 is not 64-bit on this platform!");

#endif // ACRIS_LIB_STDTYPES_HPP
