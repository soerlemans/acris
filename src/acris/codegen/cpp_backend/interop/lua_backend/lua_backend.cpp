#include "lua_backend.hpp"

// STL Includes:
#include <format>

namespace codegen::cpp_backend::interop::lua_backend {
LuaBackend::LuaBackend(): m_ss{}, m_module{}, m_symbols{}
{
  // m_symbols.reserve();
}

auto LuaBackend::backend_id() const -> std::string_view
{
  return {"lua"};
}

auto LuaBackend::prologue() -> std::string
{
  std::stringstream ss;

  ss << "// Lua binding Includes:\n";
  ss << "#include <lauxlib.h>\n";
  ss << "#include <lualib.h>\n\n";

  return ss.str();
}

auto LuaBackend::register_module(const std::string_view t_module) -> void
{
  m_module = t_module;
}

auto LuaBackend::register_constant(const std::string_view t_id) -> void
{
  m_symbols.emplace_back(std::string{t_id}, ExportSymbolType::CONSTANT);
}

auto LuaBackend::register_variable(const std::string_view t_id) -> void
{
  m_symbols.emplace_back(std::string{t_id}, ExportSymbolType::VARIABLE);
}

auto LuaBackend::register_function(const std::string_view t_id) -> void
{
  m_symbols.emplace_back(std::string{t_id}, ExportSymbolType::FUNCTION);
}

auto LuaBackend::epilogue() -> std::string
{
  std::stringstream ss;

  // TODO: Inject module name.
  ss << R"(extern "C" {)" << '\n';
  ss << "int luaopen_mylib(lua_State *L) {\n";
  ss << "\treturn 1;\n";
  ss << "}\n";
  ss << "}\n";

  return ss.str();
}
} // namespace codegen::cpp_backend::interop::lua_backend
