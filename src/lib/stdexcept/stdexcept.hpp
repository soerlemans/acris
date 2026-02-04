#ifndef ACRIS_LIB_STDEXCEPT_STDEXCEPT_HPP
#define ACRIS_LIB_STDEXCEPT_STDEXCEPT_HPP

// STL Includes:
#include <format>

// Local Includes:
#include "bad_any_cast.hpp"
#include "exception.hpp"
#include "invalid_argument.hpp"
#include "logic_error.hpp"
#include "runtime_error.hpp"
#include "todo.hpp"
#include "unexpected_monostate.hpp"
#include "unexpected_nullptr.hpp"

namespace lib::stdexcept {
template<typename ExceptType = lib::stdexcept::Exception, typename... Args>
inline auto throwf(const std::string t_fmt, Args&&... t_args) -> void
{
  const auto msg{std::format(t_fmt, std::forward<Args>(t_args)...)};

  throw ExceptType{msg};
}
} // namespace lib::stdexcept

#endif // ACRIS_LIB_STDEXCEPT_STDEXCEPT_HPP
