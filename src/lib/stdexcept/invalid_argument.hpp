#ifndef ACRIS_LIB_STDEXCEPT_INVALID_ARGUMENT_HPP
#define ACRIS_LIB_STDEXCEPT_INVALID_ARGUMENT_HPP

// Local Includes:
#include "exception.hpp"

namespace lib::stdexcept {
class InvalidArgument : public Exception {
  public:
  InvalidArgument(std::string_view t_msg);

  virtual ~InvalidArgument() = default;
};

// Functions:
template<typename... Args>
inline auto throw_invalid_argument(Args&&... t_args) -> void
{
  throw_exception<InvalidArgument>(std::forward<Args>(t_args)...);
}
} // namespace lib::stdexcept

#endif // ACRIS_LIB_STDEXCEPT_INVALID_ARGUMENT_HPP
