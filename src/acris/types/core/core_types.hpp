#ifndef ACRIS_ACRIS_TYPES_CORE_CORE_TYPES_HPP
#define ACRIS_ACRIS_TYPES_CORE_CORE_TYPES_HPP

// STL Includes:
#include <iosfwd>
#include <map>
#include <string>

// Absolute Includes:
#include "lib/stdtypes.hpp"

// Local Includes:
#include "type_variant.hpp"

namespace types::core {
// Aliases:
using MemberMap = std::map<std::string, TypeVariant>;
using MethodMap = std::map<std::string, TypeVariant>;

// Structs:
// TODO: use VarTypePtr and FnTypePtr in combination with a map?
struct StructType {
  std::string m_identifier;

  MemberMap m_members;
  MethodMap m_methods;

  auto native_type() const -> core::NativeTypeOpt;
};

struct FnType {
  TypeList m_params;
  TypeVariant m_return_type;

  auto native_type() const -> core::NativeTypeOpt;
};

struct PointerType {
  TypeVariant m_type;
  u8 m_indirection;
  bool m_readonly;

  auto native_type() const -> core::NativeTypeOpt;
};

struct ArrayType {
  TypeVariant m_type;
  std::size_t m_size;

  auto native_type() const -> core::NativeTypeOpt;
};

struct VarType {
  TypeVariant m_type;

  auto native_type() const -> core::NativeTypeOpt;
};

// Functions:
template<typename... Args>
inline auto make_struct(Args&&... t_args) -> TypeVariant
{
  return std::make_shared<StructType>(std::forward<Args>(t_args)...);
}

template<typename... Args>
inline auto make_function(Args&&... t_args) -> TypeVariant
{
  return std::make_shared<FnType>(std::forward<Args>(t_args)...);
}

template<typename... Args>
inline auto make_variable(Args&&... t_args) -> TypeVariant
{
  return std::make_shared<VarType>(std::forward<Args>(t_args)...);
}

template<typename... Args>
inline auto make_pointer(Args&&... t_args) -> TypeVariant
{
  return std::make_shared<PointerType>(std::forward<Args>(t_args)...);
}

template<typename... Args>
inline auto make_array(Args&&... t_args) -> TypeVariant
{
  return std::make_shared<ArrayType>(std::forward<Args>(t_args)...);
}

} // namespace types::core

// Functions:
auto operator<<(std::ostream& t_os, types::core::StructTypePtr t_struct)
  -> std::ostream&;
auto operator<<(std::ostream& t_os, types::core::FnTypePtr t_fn)
  -> std::ostream&;
auto operator<<(std::ostream& t_os, types::core::PointerTypePtr t_var)
  -> std::ostream&;
auto operator<<(std::ostream& t_os, types::core::ArrayTypePtr t_ptr)
  -> std::ostream&;
auto operator<<(std::ostream& t_os, types::core::VarTypePtr t_var)
  -> std::ostream&;

auto operator<<(std::ostream& t_os, const types::core::TypeList& t_list)
  -> std::ostream&;


#endif // ACRIS_ACRIS_TYPES_CORE_CORE_TYPES_HPP
