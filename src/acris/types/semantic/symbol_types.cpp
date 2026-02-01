#include "symbol_types.hpp"

// Absolute Includes:
#include "acris/debug/log.hpp"
#include "lib/stdexcept/stdexcept.hpp"

// Using Statements:
namespace types::symbol {
// StructType:
auto StructType::resolve_result_type() const -> SymbolData
{
  using types::symbol::make_struct;

  return make_struct(*this);
}

auto StructType::native_type() const -> NativeTypeOpt
{
  return {};
}

auto StructType::type_variant() const -> TypeVariant
{
  using types::core::make_struct;

  types::core::MemberMap member_map{};
  for(auto& [id, data] : m_members) {
    member_map.emplace(id, data.type_variant());
  }

  types::core::MemberMap method_map{};
  for(auto& [id, data] : m_methods) {
    method_map.emplace(id, data.type_variant());
  }

  return {make_struct(m_identifier, member_map, method_map)};
}

// FnType:
auto FnType::resolve_result_type() const -> SymbolData
{
  using types::symbol::make_function;

  return make_function(*this);
}

auto FnType::native_type() const -> NativeTypeOpt
{
  return {};
}

auto FnType::type_variant() const -> TypeVariant
{
  using types::core::make_function;
  using types::core::TypeList;

  // TODO: Convert TypeList of @ref FnType to core::TypeList.
  TypeList params;

  return {make_function(params, m_return_type.type_variant())};
}

// PointerType:
auto PointerType::resolve_result_type() const -> SymbolData
{
  using types::symbol::make_pointer;

  // Make sure to resolve underlying type.
  return make_pointer(m_type.resolve_result_type(), m_indirection, m_readonly);
}

auto PointerType::native_type() const -> NativeTypeOpt
{
  return {std::nullopt};
}

auto PointerType::type_variant() const -> TypeVariant
{
  using types::core::make_pointer;

  return {make_pointer(m_type.type_variant(), m_indirection, m_readonly)};
}

// ArrayType:
auto ArrayType::resolve_result_type() const -> SymbolData
{
  using types::symbol::make_array;

  // Make sure to resolve underlying type.
  return make_array(m_type.resolve_result_type(), m_size);
}

auto ArrayType::native_type() const -> NativeTypeOpt
{
  return {std::nullopt};
}

auto ArrayType::type_variant() const -> TypeVariant
{
  using types::core::make_array;

  return make_array(m_type.type_variant(), m_size);
}

// VarType:
auto VarType::is_mutable() const -> bool
{
  return (m_mutability == Mutability::MUTABLE);
}

auto VarType::resolve_result_type() const -> SymbolData
{
  // Get underlying type.
  return m_type.resolve_result_type();
}

auto VarType::native_type() const -> NativeTypeOpt
{
  return m_type.native_type();
}

auto VarType::type_variant() const -> TypeVariant
{
  using types::core::make_variable;

  return {make_variable(m_type.type_variant())};
}
} // namespace types::symbol
