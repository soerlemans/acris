#include "interop_backend_interface.hpp"

namespace codegen {
auto interopbackend2str(InteropBackend t_type) -> std::string_view
{
  switch(t_type) {
    case InteropBackend::C_INTEROP_BACKEND:
      return "C";

    case InteropBackend::PYTHON_INTEROP_BACKEND:
      return "python";

    case InteropBackend::LUA_INTEROP_BACKEND:
      return "lua";

      // OCaml has a C FFI.
    case InteropBackend::OCAML_INTEROP_BACKEND:
      return "ocaml";

    case InteropBackend::JS_INTEROP_BACKEND:
      return "javascript";

    default:
      throw std::invalid_argument{
        "interopbackend2str() could not convert BackendType to string."};
      break;
  }

  return {};
}
} // namespace codegen

// Operators:
auto operator<<(std::ostream& t_os, const codegen::InteropBackend t_type)
  -> std::ostream&
{
  using codegen::interopbackend2str;

  t_os << interopbackend2str(t_type);

  return t_os;
}
