#include "lua_backend.hpp"

// STL Includes:
#include <format>

// Project Includes:
#include "acris/debug/log.hpp"
#include "acris/diagnostic/diagnostic.hpp"

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
  ss << "#include <lua.hpp>\n";

  return ss.str();
}

auto LuaBackend::register_module(const std::string_view t_module) -> void
{
  using diagnostic::throw_diagnostic;
  using diagnostic::throwf_diagnostic;

  if(t_module.empty()) {
    throw_diagnostic(
      "Empty module name was passed to Lua interopability backend!");
  }

  // Check first char.
  if(!std::isalpha((uchar)t_module.front())) {
    throwf_diagnostic("First character for Lua module name, must be "
                      "alphabetic (module name {})!",
                      t_module);
  }

  // Skip first char.
  for(const auto ch : t_module.substr(1)) {
    // if(isalum() && isspace())
    if(!std::isalnum((uchar)ch) && ch != '_') {
      throwf_diagnostic(
        "Lua module name may only contain alphabetics, numerals and "
        "underscores {} (violating character {})!",
        t_module, ch);
    }
  }

  DBG_INFO("Lua module registered: ", t_module);

  m_module = t_module;
}

auto LuaBackend::register_constant(const std::string_view t_id,
                                   VarTypePtr t_var) -> void
{
  DBG_INFO("Lua function registered: ", t_id);

  m_symbols.emplace_back(std::string{t_id}, ExportSymbolType::CONSTANT, t_var);
}

auto LuaBackend::register_variable(const std::string_view t_id,
                                   VarTypePtr t_var) -> void
{
  DBG_INFO("Lua function registered: ", t_id);

  m_symbols.emplace_back(std::string{t_id}, ExportSymbolType::VARIABLE, t_var);
}

auto LuaBackend::register_function(const std::string_view t_id, FnTypePtr t_fn)
  -> void
{
  DBG_INFO("Lua function registered: ", t_id);

  m_symbols.emplace_back(std::string{t_id}, ExportSymbolType::FUNCTION, t_fn);
}

auto LuaBackend::epilogue() -> std::string
{
  std::stringstream ss;

  for(const auto& sym : m_symbols) {
    if(sym.m_type == ExportSymbolType::FUNCTION) {
      FnTypePtr ptr{sym.m_type_ctx.as_function()};

      // TODO: Generate proper bindings, instead of this lazy stuff.
      ss << std::format("int lbind_{}", sym.m_id) << '(';

      ss << "LuaState* t_state"; // Mandatory.

      // for() {
      // ss << ", " << <param>;
      //}

      ss << ")\n" << "{\n";

      // TODO: Fix shitty prototype coding.
      ss << '\t' << "return 0;\n";

      // TODO:
      ss << "}\n";
    }
  }

  ss << "static const struct luaL_Reg exported_lua_funcs[] = {\n";
  for(const auto& sym : m_symbols) {
    if(sym.m_type == ExportSymbolType::FUNCTION) {
      FnTypePtr ptr{sym.m_type_ctx.as_function()};

      // TODO: Generate proper bindings, instead of this lazy stuff.
      ss << std::format(R"({{"{}", &lbind_{} }},)", sym.m_id, sym.m_id) << '\n';
    }
  }

  ss << "{NULL, NULL}\n";
  ss << "};\n";

  // TODO: Inject module name.
  ss << R"(extern "C" {)" << '\n';
  ss << std::format("int luaopen_{}(lua_State *L) {{\n", m_module);
  ss << '\t' << "luaL_newlib(L, exported_lua_funcs);\n";
  ss << '\t' << "return 1;\n";
  ss << "}\n";
  ss << "}\n";

  return ss.str();
}
} // namespace codegen::cpp_backend::interop::lua_backend
