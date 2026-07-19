#include "python_backend.hpp"

// STL Includes:
#include <format>

namespace codegen::cpp_backend::interop::python_backend {
PythonBackend::PythonBackend(): m_ss{}, m_module{}, m_symbols{}
{
  // m_symbols.reserve();
}

auto PythonBackend::backend_id() const -> std::string_view
{
  return {"python"};
}

auto PythonBackend::prologue() -> std::string
{
  std::stringstream ss;

  ss << "// Pybind Includes:\n";
  ss << "#include <pybind11/pybind11.h>\n\n";

  return ss.str();
}

auto PythonBackend::register_module(std::string_view t_module) -> void
{
  m_module = t_module;
}

auto PythonBackend::register_constant(const std::string_view t_id,
                                      VarTypePtr t_var) -> void
{
  m_symbols.emplace_back(std::string{t_id}, ExportSymbolType::CONSTANT);
}

auto PythonBackend::register_variable(const std::string_view t_id,
                                      VarTypePtr t_var) -> void
{
  m_symbols.emplace_back(std::string{t_id}, ExportSymbolType::VARIABLE);
}

auto PythonBackend::register_function(const std::string_view t_id,
                                      FnTypePtr t_fn) -> void
{
  m_symbols.emplace_back(std::string{t_id}, ExportSymbolType::FUNCTION);
}

auto PythonBackend::epilogue() -> std::string
{
  std::stringstream ss;

  // TODO: Set the module name based on package/module name or macro value.
  ss << std::format("PYBIND11_MODULE({}, mod) {{\n", m_module);
  ss << "namespace py = pybind11;\n";
  ss << R"(mod.doc() = "Acris program exported symbols.";)" << '\n';

  for(const auto& export_symbol : m_symbols) {
    const auto& [id, type] = export_symbol;
    switch(type) {
      case ExportSymbolType::CONSTANT:
        ss << std::format(R"(mod.attr("{}") = {})", id, id) << '\n';
        break;

      case ExportSymbolType::VARIABLE:
        ss << std::format(R"(mod.attr("{}") = &{})", id, id) << '\n';
        break;

      case ExportSymbolType::FUNCTION:
        ss << std::format(R"(mod.def("{}", &{}, "Acris exported function.");)",
                          id, id)
           << '\n';
        break;

      default:
        // TODO: Error handle.
        break;
    }
  }

  ss << "}\n";

  return ss.str();
}
} // namespace codegen::cpp_backend::interop::python_backend
