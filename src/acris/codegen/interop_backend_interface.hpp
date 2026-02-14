#ifndef ACRIS_ACRIS_CODEGEN_INTEROP_BACKEND_INTERFACE_HPP
#define ACRIS_ACRIS_CODEGEN_INTEROP_BACKEND_INTERFACE_HPP

// STL Includes:
#include <memory>
#include <string>
#include <vector>

// Absolute Includes:
#include "acris/ast/node/fdecl.hpp"
#include "lib/stdtypes.hpp"

namespace codegen {
// Using Statements:
using namespace ast;

// Using Declarations:
using node::NodePtr;

// Forward Declarations:
class InteropBackendInterface;
enum class InteropBackend;

// Aliases:
using InteropSelectors = std::vector<InteropBackend>;
using InteropBackendPtr = std::shared_ptr<InteropBackendInterface>;

// Enums:
/*!
 * Keeps track of which interop backends exist.
 * Not all interop backends enums need to be supported.
 * By every codegeneration backend.
 */
enum class InteropBackend {
  C_INTEROP_BACKEND,
  PYTHON_INTEROP_BACKEND,
  LUA_INTEROP_BACKEND,
  JS_INTEROP_BACKEND,
};

// Classes:
/*!
 * Interface used for creating interop between different programming languages.
 */
class InteropBackendInterface {
  public:
  InteropBackendInterface() = default;

  virtual ~InteropBackendInterface() = default;
};

auto interopbackend2str(InteropBackend t_type) -> std::string_view;
} // namespace codegen

// Functions:
auto operator<<(std::ostream& t_os, codegen::InteropBackend t_type)
  -> std::ostream&;

#endif // ACRIS_ACRIS_CODEGEN_INTEROP_BACKEND_INTERFACE_HPP
