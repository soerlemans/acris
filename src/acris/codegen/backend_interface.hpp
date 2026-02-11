#ifndef ACRIS_ACRIS_CODEGEN_BACKEND_INTERFACE_HPP
#define ACRIS_ACRIS_CODEGEN_BACKEND_INTERFACE_HPP

// STL Includes:
#include <memory>

// Absolute Includes:
#include "acris/ast/visitor/node_visitor.hpp"
#include "acris/codegen/interop_backend_interface.hpp"
#include "acris/mir/mir.hpp"
#include "lib/filesystem.hpp"

namespace codegen {
// Using:
namespace node = ast::node;

using ast::node::NodePtr;
using mir::ModulePtr;

// Forward Declarations:
class BackendInterface;

// Aliases:
using BackendPtr = std::shared_ptr<BackendInterface>;

namespace fs = std::filesystem;

// Enums:
enum class BackendType {
  LLVM_BACKEND,
  JS_BACKEND,
  CPP_BACKEND,
  C_BACKEND,
};

// Structs:
/*!
 * Utility structure packing all required parameters for compiling.
 */
struct CompileParams {
  NodePtr m_ast;
  ModulePtr m_mir;

  fs::path m_build_dir;
  fs::path m_source_path;
};

// Classes:
/*!
 * This is an interface for communicating with backends in a generic way.
 */
class BackendInterface {
  public:
  BackendInterface() = default;

  /*!
   * How an interop backend is added is backend specific.
   * So this is a shared factory method for nested interop backends.
   */
  virtual auto register_interop_backend(InteropBackendType t_type) -> void = 0;

  /*!
   * Check if MIR is required for compilation.
   */
  virtual auto requires_mir() -> bool = 0;

  //! Compile the AST for the selected backend.
  virtual auto compile(CompileParams& t_params) -> void = 0;

  virtual ~BackendInterface() = default;
};

// Functions:
// TODO: Implement with a unique_ptr or something similar.
[[nodiscard("Pure method must use results.")]]
auto select_backend(BackendType t_selector) -> BackendPtr;

[[nodiscard("Pure method must use results.")]]
auto backendtype2str(BackendType t_type) -> std::string_view;
} // namespace codegen

auto operator<<(std::ostream& t_os, codegen::BackendType t_type)
  -> std::ostream&;


#endif // ACRIS_ACRIS_CODEGEN_BACKEND_INTERFACE_HPP
