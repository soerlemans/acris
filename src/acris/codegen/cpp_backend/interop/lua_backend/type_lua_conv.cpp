#include "type_lua_conv.hpp"

// STL Includes:
#include <memory>

// Absolute Includes:
#include "acris/ast/node/include_nodes.hpp"
#include "lib/overload.hpp"
#include "lib/stdexcept/stdexcept.hpp"

// Macros:
#define MATCH(t_key, t_value) \
  case NativeType::t_key:     \
    str = t_value;            \
    break

namespace {
// Using:
using codegen::cpp_backend::interop::lua_backend::LuaQueryError;
using codegen::cpp_backend::interop::lua_backend::LuaQueryErrorType;
using codegen::cpp_backend::interop::lua_backend::LuaQueryOp;
using codegen::cpp_backend::interop::lua_backend::LuaQueryResult;
using codegen::cpp_backend::interop::lua_backend::QuerySpec;
using codegen::cpp_backend::interop::lua_backend::type_lua_conv;
using lib::stdexcept::InvalidArgument;
using lib::stdexcept::throwf;
using types::core::ArrayTypePtr;
using types::core::EnumTypePtr;
using types::core::FnTypePtr;
using types::core::NativeType;
using types::core::nativetype2str;
using types::core::PointerTypePtr;
using types::core::StructTypePtr;
using types::core::VarTypePtr;

// Functions:
inline auto native_type_lua_conv_check(const NativeType t_type,
                                       const QuerySpec& t_spec)
  -> LuaQueryResult
{
  std::string str{};

  // When checking we are extracting a value and we just flat out error.
  // On any narrowing cast.
  switch(t_type) {
    case NativeType::F64:
      str = "luaL_checknumber";
      break;

    case NativeType::I64:
      str = "luaL_checkinteger";
      break;

    case NativeType::U64:
      str = "(uint64_t)luaL_checkinteger";
      break;

    case NativeType::CSTR:
      str = "luaL_checkstring";
      break;

    case NativeType::BOOL:
      // Warning: Lua interop backend generated function.
      str = "luaL_checkboolean";
      break;

      // TODO: Convert to string equivalent for error message.
    case NativeType::VOID:
      [[fallthrough]];

    case NativeType::F32:
      [[fallthrough]];

    case NativeType::CHAR:
      [[fallthrough]];
      // We dont do char functions they are too inefficient.

    case NativeType::INT:
      [[fallthrough]];
    case NativeType::I8:
      [[fallthrough]];
    case NativeType::I16:
      [[fallthrough]];
    case NativeType::I32:
      [[fallthrough]];
    case NativeType::ISIZE:
      [[fallthrough]];

    case NativeType::UINT:
      [[fallthrough]];
    case NativeType::U8:
      [[fallthrough]];
    case NativeType::U16:
      [[fallthrough]];
    case NativeType::U32:
      [[fallthrough]];
    case NativeType::USIZE:
      // TODO: Cleanup to be non shit error message.
      return std::unexpected{
        LuaQueryError{
                      LuaQueryErrorType::UNSUPPORTED,
                      std::format(
            "Native type cant be converted to Lua check function! ({})", nativetype2str(t_type))}
      };

    default:
      throwf<InvalidArgument>("NativeType unhandled ({}).",
                              nativetype2str(t_type));
      break;
  }

  return str;
}

inline auto native_type_lua_conv_push(const NativeType t_type,
                                      const QuerySpec& t_spec) -> LuaQueryResult
{
  std::string str{};

  // FIXME: Currently the C++ fixed width floating point types.
  // Are not yet supported by clang libc++.
  // So for now just error out, on these.
  // Possibly print currently unsupported?
  switch(t_type) {
    case NativeType::F32:
      [[fallthrough]];
    case NativeType::F64:
      str = "luaL_pushnumber";
      break;

      // We can push smaller values to larger containers but not vice versa.
    case NativeType::INT:
      [[fallthrough]];
    case NativeType::I8:
      [[fallthrough]];
    case NativeType::I16:
      [[fallthrough]];
    case NativeType::I32:
      [[fallthrough]];
    case NativeType::I64:
      [[fallthrough]];
    case NativeType::ISIZE:
      str = "lua_pushinteger";
      break;

    case NativeType::UINT:
      [[fallthrough]];
    case NativeType::U8:
      [[fallthrough]];
    case NativeType::U16:
      [[fallthrough]];
    case NativeType::U32:
      [[fallthrough]];
    case NativeType::U64:
      [[fallthrough]];
    case NativeType::USIZE:
      str = "lua_pushinteger";
      break;

    case NativeType::CSTR:
      str = "luaL_pushstring";
      break;

    case NativeType::BOOL:
      // Warning: Lua interop backend generated function.
      str = "luaL_pushboolean";
      break;

    case NativeType::VOID:

    case NativeType::CHAR:
      [[fallthrough]];

      // TODO: Cleanup to be non shit error message.
      return std::unexpected{
        LuaQueryError{LuaQueryErrorType::UNSUPPORTED,
                      "Native type cant be converted to Lua for reasons!"}
      };
      break;

    default:
      throwf<InvalidArgument>("NativeType could not be converted to Lua: ",
                              nativetype2str(t_type));
      break;
  }

  return str;
}

/*!
 * @warning Make sure that <cstdint> is included for the fixed width integers.
 */
inline auto native_type_lua_conv(const NativeType t_type,
                                 const QuerySpec& t_spec) -> LuaQueryResult
{
  std::string str{};

  switch(t_spec.m_op) {
    case LuaQueryOp::CHECK:
      return native_type_lua_conv_check(t_type, t_spec);

    case LuaQueryOp::PUSH:
      return native_type_lua_conv_push(t_type, t_spec);

    default:
      throwf<InvalidArgument>("Invalid query option.");
      break;
  }

  return str;
}

inline auto enum_conv(const EnumTypePtr& t_enum, const QuerySpec& t_spec)
  -> LuaQueryResult
{
  return t_enum->m_identifier;
}

inline auto struct_conv(const StructTypePtr& t_struct, const QuerySpec& t_spec)
  -> LuaQueryResult
{
  return std::unexpected{
    LuaQueryError{LuaQueryErrorType::UNSUPPORTED, "Not implemented."}
  };
}

// TODO: Figure out how to deal with this one.
inline auto fn_conv([[maybe_unused]] const FnTypePtr& t_type,
                    const QuerySpec& t_spec) -> LuaQueryResult
{
  return std::unexpected{
    LuaQueryError{LuaQueryErrorType::UNSUPPORTED, "Not implemented."}
  };
}

inline auto pointer_conv(const PointerTypePtr& t_ptr, const QuerySpec& t_spec)
  -> LuaQueryResult
{
  return std::unexpected{
    LuaQueryError{LuaQueryErrorType::UNSUPPORTED, "Not implemented."}
  };
}

inline auto array_conv(const ArrayTypePtr& t_arr, const QuerySpec& t_spec)
  -> LuaQueryResult
{
  return std::unexpected{
    LuaQueryError{LuaQueryErrorType::UNSUPPORTED, "Not implemented."}
  };
}

// Could be used for decltype() or similar.
inline auto var_conv(const VarTypePtr& t_var, const QuerySpec& t_spec)
  -> LuaQueryResult
{
  // Just resolve base type.
  auto result{type_lua_conv(t_var->m_type, t_spec)};
  if(result.has_value()) {
    return std::format("{}", result.value());
  }

  return result;
}
} // namespace

namespace codegen::cpp_backend::interop::lua_backend {
// Using Statements:
NODE_USING_ALL_NAMESPACES()

auto type_lua_conv(const TypeVariant& t_type, const QuerySpec& t_spec)
  -> LuaQueryResult
{
  using lib::Overload;

  // clang-format off
  return std::visit(
					Overload{
						[&](const NativeType t_native) {
							return native_type_lua_conv(t_native, t_spec);
						},
						[&](const EnumTypePtr& t_enum) {
							return enum_conv(t_enum, t_spec);
						},
						[&](const StructTypePtr& t_struct) {
							return struct_conv(t_struct, t_spec);
						},
						[&](const FnTypePtr& t_fn) {
							return fn_conv(t_fn, t_spec);
						},
						[&](const PointerTypePtr& t_ptr) {
							return pointer_conv(t_ptr, t_spec);
						},
						[&](const ArrayTypePtr& t_arr) {
							return array_conv(t_arr, t_spec);
						},
						[&](const VarTypePtr& t_var) {
							return var_conv(t_var, t_spec);
						}},
					t_type);
	// clang-format off
}
} // namespace codegen::cpp_backend::interop::lua_backend
