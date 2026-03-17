#ifndef ACRIS_ACRIS_CODEGEN_CPP_BACKEND_INTEROP_PYTHON_BACKEND_PYTHON_BACKEND_HPP
#define ACRIS_ACRIS_CODEGEN_CPP_BACKEND_INTEROP_PYTHON_BACKEND_PYTHON_BACKEND_HPP

// STL Includes:
#include <sstream>
#include <vector>

// Absolute Includes:
#include "acris/codegen/cpp_backend/interop/cpp_interop_backend_interface.hpp"

namespace codegen::cpp_backend::interop::python_backend {
// Forward Declarations:
struct ExportSymbol;

// Aliases:
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
};

// Classes:
/*!
 * Generate pythong bindings for use with pybind11.
 * TODO: Describe usage generate(), dont run on whole ast.
 */
class PythonBackend : public CppInteropBackendInterface {
  private:
  std::stringstream m_ss;

  ExportSymbols m_symbols;

  public:
  PythonBackend();

  auto backend_id() const -> std::string_view override;

  auto prologue() -> std::string override;

  auto register_constant(std::string_view t_id) -> void override;
  auto register_variable(std::string_view t_id) -> void override;
  auto register_function(std::string_view t_id) -> void override;

  auto epilogue() -> std::string override;

  virtual ~PythonBackend() = default;
};
} // namespace codegen::cpp_backend::interop::python_backend

#endif // ACRIS_ACRIS_CODEGEN_CPP_BACKEND_INTEROP_PYTHON_BACKEND_PYTHON_BACKEND_HPP
