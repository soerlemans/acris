#ifndef ACRIS_ACRIS_CODEGEN_CPP_BACKEND_TYPE2CPP_HPP
#define ACRIS_ACRIS_CODEGEN_CPP_BACKEND_TYPE2CPP_HPP

// STL Includes:
#include <expected>
#include <string>
#include <string_view>

// Absolute Includes:
#include "acris/ast/node/fdecl.hpp"
#include "acris/types/core/core.hpp"

namespace codegen::cpp_backend::interop::lua_backend {
// Forward Decl:
struct LuaQueryError;

// Aliases:
using types::core::TypeVariant;

using LuaQueryResult = std::expected<std::string, LuaQueryError>;


enum class LuaQueryErrorType {
  UNSUPPORTED
};

enum class LuaQueryOp {
  CHECK,
  PUSH,
  NEW
};

struct LuaQueryError {
  LuaQueryErrorType m_type;
  std::string m_msg;
};

// TODO: Improve API for converting a type specification into C++.
// Currently the way for having to pass the identifier for arrays is awkward.
/*!
 * Some types in C++ make the identifier part of the type expression.
 */
struct QuerySpec {
  LuaQueryOp m_op;
};

/*!
 * Convert the @ref TypeVariant to Lua matching code.
 */
auto type_lua_conv(const TypeVariant& t_type, const QuerySpec& t_spec)
  -> LuaQueryResult ;
} // namespace codegen::cpp_backend::interop::lua_backend

#endif // ACRIS_ACRIS_CODEGEN_CPP_BACKEND_TYPE2CPP_HPP
