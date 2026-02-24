#include "symbol_data.hpp"

// STL Includes:
#include <format>
#include <functional>

// Absolute Includes:
#include "acris/debug/log.hpp"
#include "lib/is_any_of.hpp"
#include "lib/overload.hpp"
#include "lib/stdexcept/stdexcept.hpp"

// Local Includes:
#include "symbol_types.hpp"

// Internal:
namespace {
// TODO: Move somewhere proper.
// Used to guarentee evaluation in else branches, to trigger static_assert().
template<typename T>
inline constexpr bool always_false = false;

/*!
 * Needs to be a struct because @ref Overload only accepts functors.
 * The handler is used to catch the error state where @ref std::monostate is.
 * Set when its not supposed to be.
 * We need to be able to specifically set the return type.
 */
template<typename T = void>
struct MonoStateHandler {
  auto operator()(std::monostate) -> T
  {
    using lib::stdexcept::throw_unexpected_monostate;

    throw_unexpected_monostate("Illegal occurrence of std::monostate.");

    // std::unreachable();

    // Will never be reached.
    return T{};
  }
};

// Functions:
/*!
 * Check if a @ref std::shared_ptr is a nullptr or not.
 * If the given ptr is a nullptr we should throw.
 * Only use in cases where you are certain a nullptr should never happen.
 */
template<typename T>
auto nullptr_check(const std::string_view t_str,
                   const std::shared_ptr<T>& t_ptr) -> void
{
  using lib::stdexcept::throw_unexpected_nullptr;

  if(!t_ptr) {
    const auto msg{std::format("{} ptr is nullptr!", t_str)};

    throw_unexpected_nullptr(msg);
  }
}
} // namespace

namespace types::symbol {
// Methods:
auto SymbolData::is_enum() const -> bool
{
  return std::holds_alternative<EnumTypePtr>(*this);
}

auto SymbolData::is_struct() const -> bool
{
  return std::holds_alternative<StructTypePtr>(*this);
}

auto SymbolData::is_ptr() const -> bool
{
  return std::holds_alternative<PointerTypePtr>(*this);
}

auto SymbolData::is_array() const -> bool
{
  return std::holds_alternative<ArrayTypePtr>(*this);
}

auto SymbolData::as_enum() const -> EnumTypePtr
{
  return std::get<EnumTypePtr>(*this);
}

auto SymbolData::as_struct() const -> StructTypePtr
{
  return std::get<StructTypePtr>(*this);
}

auto SymbolData::as_function() const -> FnTypePtr
{
  return std::get<FnTypePtr>(*this);
}

auto SymbolData::as_ptr() const -> PointerTypePtr
{
  return std::get<PointerTypePtr>(*this);
}

auto SymbolData::as_array() const -> ArrayTypePtr
{
  return std::get<ArrayTypePtr>(*this);
}

auto SymbolData::as_var() const -> VarTypePtr
{
  return std::get<VarTypePtr>(*this);
}

auto SymbolData::is_mutable() const -> bool
{
  using lib::Overload;

  bool result{false};

  const auto var_type{[&](const VarTypePtr& t_data) {
    nullptr_check("Variable", t_data);

    return (t_data->m_mutability == Mutability::MUTABLE);
  }};

  const auto immutable_types{[]([[maybe_unused]] const auto& t_data) {
    return false;
  }};

  result = std::visit(
    Overload{var_type, immutable_types, MonoStateHandler<bool>{}}, *this);

  return result;
}

auto SymbolData::resolve_result_type() const -> SymbolData
{
  using lib::Overload;

  SymbolData data{};

  const auto native{[&](const NativeType t_type) -> SymbolData {
    return SymbolData{t_type};
  }};

  const auto rest{[&]<typename T>(const std::shared_ptr<T>& t_ptr) {
    nullptr_check("Resolve result type", t_ptr);

    return t_ptr->resolve_result_type();
  }};


  data =
    std::visit(Overload{native, rest, MonoStateHandler<SymbolData>{}}, *this);

  return data;
}

auto SymbolData::native_type() const -> NativeTypeOpt
{
  using lib::Overload;

  NativeTypeOpt opt{};

  const auto native{[&](const NativeType t_type) -> NativeTypeOpt {
    return t_type;
  }};

  const auto rest{[&]<typename T>(const std::shared_ptr<T>& t_data) {
    nullptr_check("Non native", t_data);

    // Note this is a recursive call.
    return t_data->native_type();
  }};

  opt = std::visit(Overload{native, rest, MonoStateHandler<NativeTypeOpt>{}},
                   *this);

  return opt;
}

auto SymbolData::type_variant() const -> TypeVariant
{
  using lib::Overload;

  TypeVariant variant;

  const auto native{[&](const NativeType t_type) -> TypeVariant {
    return {t_type};
  }};

  // Calls variant() method of types defined in symbol_types.hpp header.
  const auto rest{[&]<typename T>(const std::shared_ptr<T>& t_data) {
    nullptr_check("Non native", t_data);

    return t_data->type_variant();
  }};

  variant =
    std::visit(Overload{native, rest, MonoStateHandler<TypeVariant>{}}, *this);

  return variant;
}

auto SymbolData::operator==(const SymbolData& t_rhs) const -> bool
{
  bool equal{false};
  const auto& lhs{*this};

  equal = std::visit(
    [](const auto& t_l, const auto& t_r) -> bool {
      using L = std::decay_t<decltype(t_l)>;
      using R = std::decay_t<decltype(t_r)>;

      if constexpr(!std::is_same_v<L, R>) {
        // Guard clause make sure the types are the same.
        // Or we just return false.
        return false;
      } else if constexpr(std::is_same_v<L, NativeType>) {
        // NativeType is just a simple compare.
        return (t_l == t_r);
        // clang-format off
      } else if constexpr(
				 lib::IsAnyOf<L,
				   EnumTypePtr,
					 StructTypePtr, FnTypePtr,
           PointerTypePtr, ArrayTypePtr, VarTypePtr
				 >) {
        // clang-format on
        if(t_l && t_r) {
          // Compare resolved pointers.
          return (*t_l == *t_r);
        } else {
          // Return false on any nullptr.
          return false;
        }
      } else if constexpr(std::is_same_v<L, std::monostate>) {
        // We should try to avoid throwing in operator==.
        // So just compare std::monostate and return.
        return (t_l == t_r);
      } else {
        static_assert(always_false<L>, "Unhandled type.");
      }

      return false;
    },
    lhs, t_rhs);

  return equal;
}

auto SymbolData::operator!=(const SymbolData& t_rhs) const -> bool
{
  const auto& lhs{*this};

  // Reuse operator== logic.
  return !(lhs == t_rhs);
}
} // namespace types::symbol

// Functions:
auto operator<<(std::ostream& t_os, const types::symbol::SymbolData& t_data)
  -> std::ostream&
{
  std::visit(
    [&](auto&& t_v) {
      t_os << t_v;
    },
    t_data);

  return t_os;
}
