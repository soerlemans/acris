#ifndef ACRIS_ACRIS_TYPES_SEMANTIC_SYMBOL_DATA_HPP
#define ACRIS_ACRIS_TYPES_SEMANTIC_SYMBOL_DATA_HPP

// STL Includes:
#include <memory>
#include <type_traits>
#include <variant>

// Absolute Includes:
#include "acris/types/core/core.hpp"

// Local Includes:
#include "symbol.hpp"

/*!
 * @file
 *
 * TODO: Describe the differences between the @ref TypeVariant.
 * And @ref SymbolData.
 */

namespace types::symbol {
// Using Statements:
using types::core::NativeType;
using types::core::NativeTypeOpt;
using types::core::TypeVariant;

// Aliases:
// clang-format off
using Variant = std::variant<
	std::monostate,
	NativeType,
	EnumTypePtr,
	StructTypePtr,
	FnTypePtr,
	PointerTypePtr,
	ArrayTypePtr,
	VarTypePtr
>;
// clang-format on

// Classes:
// FIXME: We probably should probably not inherit from Variant.
// Instead we should prefer composition.
/*!
 * Contains all data relating to a symbol.
 * This is different from @ref TypeVariant.
 * As this also constitues data relating to a symbol.
 * Like constness and other helper methods.
 * @ref TypeVariant has a separate type tree.
 * And you need to convert between these two.
 */
class SymbolData : public Variant {
  public:
  // Use the constructors of the parent class.
  using Variant::Variant;

  //! Verify if a symbol is a NativeType.
  auto is_monostate() const -> bool;

  //! Verify if a symbol is a NativeType.
  auto is_native() const -> bool;

  //! Verify if a symbol is an enum type.
  auto is_enum() const -> bool;

  //! Verify if a symbol is a struct type.
  auto is_struct() const -> bool;

  //! Verify if a symbol is a function type.
  auto is_fn() const -> bool;

  //! Verify if a symbol is a pointer type.
  auto is_ptr() const -> bool;

  //! Verify if a symbol is an array type.
  auto is_array() const -> bool;

  //! Verify if a symbol is a variable type.
  auto is_var() const -> bool;

  // Only use these methods if you are sure about the underlying type.
  auto as_enum() const -> EnumTypePtr;
  auto as_struct() const -> StructTypePtr;
  auto as_function() const -> FnTypePtr;
  auto as_ptr() const -> PointerTypePtr;
  auto as_array() const -> ArrayTypePtr;
  auto as_var() const -> VarTypePtr;

  auto tag_str() const -> std::string_view;


  //! Verify if a symbol is immutable.
  auto is_mutable() const -> bool;

  /*!
   * Resolves a @ref SymbolData to its underlying result type.
   * Used to compare the results of expressions for type validation purposes.
   */
  auto resolve_result_type() const -> SymbolData;

  /*!
   * Resolves a Symbol's data  to a @ref NativeType if possible.
   * Usefull for when dealing with native type only operations.
   * Like type promotion in arithmetic/comparison operations.
   */
  auto native_type() const -> NativeTypeOpt;

  /*!
   * Used to convert @ref SymbolData to a  @ref core::TypeVariant.
   * The type tree for @ref SymbolData contains extra information.
   * Like if a symbol is const or pure.
   * When we annotate the AST we dont want to include this information.
   * So this information is stripped.
   */
  auto type_variant() const -> TypeVariant;

  auto operator==(const SymbolData& t_rhs) const -> bool;
  auto operator!=(const SymbolData& t_rhs) const -> bool;

  virtual ~SymbolData() = default;
};
} // namespace types::symbol

// Functions:
auto operator<<(std::ostream& t_os, const types::symbol::SymbolData& t_data)
  -> std::ostream&;

#endif // ACRIS_ACRIS_TYPES_SEMANTIC_SYMBOL_DATA_HPP
