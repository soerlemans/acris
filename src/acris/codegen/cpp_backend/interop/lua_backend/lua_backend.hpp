#ifndef ACRIS_ACRIS_CODEGEN_CPP_BACKEND_INTEROP_LUA_BACKEND_LUA_BACKEND_HPP
#define ACRIS_ACRIS_CODEGEN_CPP_BACKEND_INTEROP_LUA_BACKEND_LUA_BACKEND_HPP

// STL Includes:
#include <sstream>
#include <vector>

// Absolute Includes:
#include "acris/codegen/cpp_backend/interop/cpp_interop_backend_interface.hpp"
#include "acris/types/core/core.hpp"

namespace codegen::cpp_backend::interop::lua_backend {
// Forward Declarations:
struct ExportSymbol;

// Aliases:
using types::core::FnTypePtr;
using types::core::TypeVariant;
using types::core::VarTypePtr;

//! Symbols that should be exported.
using ExportSymbols = std::vector<ExportSymbol>;

// Enums:
/*!
 * Symbol type that is being exported.
 */
enum class ExportSymbolType {
  FUNCTION,
  CONSTANT,
  VARIABLE
};

// Structs:
/*!
 * Struct keeping track of exported symbols regarding interoperability.
 * The identifier is a
 */
struct ExportSymbol {
  std::string m_id;
  ExportSymbolType m_type;
  TypeVariant m_type_ctx;
};

// Classes:
/*!
 * Generate pythong bindings for use with pybind11.
 * TODO: Describe usage generate(), dont run on whole ast.
 */
class LuaBackend : public CppInteropBackendInterface {
  private:
  std::stringstream m_ss;

  std::string m_module;
  ExportSymbols m_symbols;

  public:
  LuaBackend();

  auto backend_id() const -> std::string_view override;

  auto prologue() -> std::string override;

  auto register_module(std::string_view t_module) -> void override;

  auto register_constant(std::string_view t_id, VarTypePtr t_var)
    -> void override;
  auto register_variable(std::string_view t_id, VarTypePtr t_var)
    -> void override;
  auto register_function(std::string_view t_id, FnTypePtr t_fn)
    -> void override;

  auto epilogue() -> std::string override;

  virtual ~LuaBackend() = default;
};
} // namespace codegen::cpp_backend::interop::lua_backend

#endif // ACRIS_ACRIS_CODEGEN_CPP_BACKEND_INTEROP_LUA_BACKEND_LUA_BACKEND_HPP
