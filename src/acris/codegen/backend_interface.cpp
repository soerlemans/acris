#include "backend_interface.hpp"

// Relative Includes:
#include "cpp_backend/cpp_backend.hpp"
#include "llvm_backend/llvm_backend.hpp"

namespace codegen {
auto select_backend(const Backend t_selector) -> BackendPtr
{
  using cpp_backend::CppBackend;
  using llvm_backend::LlvmBackend;

  BackendPtr ptr{};

  switch(t_selector) {
    case Backend::LLVM_BACKEND: {
      ptr = std::make_shared<LlvmBackend>();
      break;
    }

    case Backend::CPP_BACKEND: {
      ptr = std::make_shared<CppBackend>();
      break;
    }

    default:
      const auto err_msg{std::format(
        "Unsupported codegeneration backend \"{}\"", backend2str(t_selector))};
      throw std::invalid_argument{err_msg};
      break;
  }

  return ptr;
}

auto backend2str(Backend t_type) -> std::string_view
{
  switch(t_type) {
    case Backend::CPP_BACKEND:
      return "cpp";

    case Backend::LLVM_BACKEND:
      return "llvm";

    default:
      throw std::invalid_argument{
        "backend2str() could not convert Backend to string."};
      break;
  }

  return {};
}

auto optimize2str(Optimize t_level) -> std::string_view
{
  switch(t_level) {
    case Optimize::NONE:
      return "none";

    case Optimize::SIZE:
      return "size";

    case Optimize::LEVEL_1:
      return "level 1";

    case Optimize::LEVEL_2:
      return "level 2";

    case Optimize::LEVEL_3:
      return "level 3";

    default:
      throw std::invalid_argument{"optimizationlevel2str() could not convert "
                                  "Optimize to string."};
      break;
  }

  return {};
}


} // namespace codegen

auto operator<<(std::ostream& t_os, codegen::Backend t_type) -> std::ostream&
{
  using codegen::backend2str;

  t_os << backend2str(t_type);

  return t_os;
}

auto operator<<(std::ostream& t_os, const codegen::Optimize t_level)
  -> std::ostream&
{
  using codegen::optimize2str;

  t_os << optimize2str(t_level);

  return t_os;
}
