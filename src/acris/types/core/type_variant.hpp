#ifndef ACRIS_ACRIS_TYPES_CORE_TYPE_VARIANT_HPP
#define ACRIS_ACRIS_TYPES_CORE_TYPE_VARIANT_HPP

// STL Includes:
#include <iosfwd>
#include <variant>

// Local Includes:
#include "fdecl.hpp"

namespace types::core {
// Using Statements:
using types::core::NativeType;

// Aliases:
// TODO: Add monostate.
using Variant = std::variant<NativeType, StructTypePtr, FnTypePtr,
                             PointerTypePtr, ArrayTypePtr, VarTypePtr>;

// Classes:
// FIXME: We probably should probably not inherit from Variant.
// Instead we should prefer composition.
/*!
 * Recursive @ref Variant tree structure that denotes the type tree of a symbol.
 * This class unlike @ref SymbolData solely holds the type information.
 * The type tree is also a separate one from @ref SymbolData.
 */
class TypeVariant : public Variant {
  public:
  // Use the constructors of the parent class.
  using Variant::Variant;

  auto is_native_type() const -> bool;
  auto is_struct() const -> bool;
  auto is_function() const -> bool;
  auto is_ptr() const -> bool;
  auto is_array() const -> bool;
  auto is_var() const -> bool;

  auto as_native_type() const -> NativeType;
  auto as_struct() const -> StructTypePtr;
  auto as_function() const -> FnTypePtr;
  auto as_ptr() const -> PointerTypePtr;
  auto as_array() const -> ArrayTypePtr;
  auto as_var() const -> VarTypePtr;

  auto native_type() const -> core::NativeTypeOpt;

  virtual ~TypeVariant() = default;
};
} // namespace types::core

// Functions:
auto operator<<(std::ostream& t_os, const types::core::TypeVariant& t_data)
  -> std::ostream&;

#endif // ACRIS_ACRIS_TYPES_CORE_TYPE_VARIANT_HPP
