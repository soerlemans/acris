#include "type_variant.hpp"

// Absolute Includes:
#include "acris/debug/log.hpp"
#include "acris/types/core/core_types.hpp"
#include "acris/types/core/native_types.hpp"
#include "lib/overload.hpp"
#include "lib/stdexcept/stdexcept.hpp"

// Local Includes:
#include "core_types.hpp"

namespace types::core {
// Methods:
auto TypeVariant::is_native_type() const -> bool
{
  return std::holds_alternative<NativeType>(*this);
}

auto TypeVariant::is_enum() const -> bool
{
  return std::holds_alternative<EnumTypePtr>(*this);
}

auto TypeVariant::is_struct() const -> bool
{
  return std::holds_alternative<StructTypePtr>(*this);
}

auto TypeVariant::is_function() const -> bool
{
  return std::holds_alternative<StructTypePtr>(*this);
}

auto TypeVariant::is_ptr() const -> bool
{
  return std::holds_alternative<PointerTypePtr>(*this);
}

auto TypeVariant::is_array() const -> bool
{
  return std::holds_alternative<ArrayTypePtr>(*this);
}

auto TypeVariant::is_var() const -> bool
{
  return std::holds_alternative<ArrayTypePtr>(*this);
}

auto TypeVariant::as_native_type() const -> NativeType
{
  return std::get<NativeType>(*this);
}

auto TypeVariant::as_enum() const -> EnumTypePtr
{
  return std::get<EnumTypePtr>(*this);
}

auto TypeVariant::as_struct() const -> StructTypePtr
{
  return std::get<StructTypePtr>(*this);
}

auto TypeVariant::as_function() const -> FnTypePtr
{
  return std::get<FnTypePtr>(*this);
}

auto TypeVariant::as_ptr() const -> PointerTypePtr
{
  return std::get<PointerTypePtr>(*this);
}

auto TypeVariant::as_array() const -> ArrayTypePtr
{
  return std::get<ArrayTypePtr>(*this);
}

auto TypeVariant::as_var() const -> VarTypePtr
{
  return std::get<VarTypePtr>(*this);
}

//! Resolves the @ref TypeVariant to a @ref NativeType if possible.
auto TypeVariant::native_type() const -> NativeTypeOpt
{
  using lib::Overload;
  using lib::stdexcept::throw_unexpected_nullptr;

  NativeTypeOpt opt;

  const auto native{[&](const NativeType t_type) -> NativeTypeOpt {
    return t_type;
  }};

  const auto methods{[&]<typename T>(const std::shared_ptr<T>& t_data) {
    if(!t_data) {
      // TODO: Improve error message.
      throw_unexpected_nullptr("ptr is nullptr!");
    }

    return t_data->native_type();
  }};

  opt = std::visit(Overload{native, methods}, *this);

  return opt;
}
} // namespace types::core

// Functions:
auto operator<<(std::ostream& t_os, const types::core::TypeVariant& t_variant)
  -> std::ostream&
{
  std::visit(
    [&](auto&& t_elem) {
      t_os << t_elem;
    },
    t_variant);

  return t_os;
}
