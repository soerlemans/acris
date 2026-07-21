#ifndef ACRIS_ACRIS_DIAGNOSTIC_TYPE_ERROR_HPP
#define ACRIS_ACRIS_DIAGNOSTIC_TYPE_ERROR_HPP

// Local Includes:
#include "diagnostic_error.hpp"

namespace diagnostic {
// Classes:
class TypeError : public DiagnosticError {
  private:
  auto format(std::string_view t_msg) -> std::string;

  public:
  TypeError(std::string_view t_msg);

  virtual ~TypeError() = default;
};

// Functions:
template<typename... Args>
inline auto throw_type_error(Args&&... t_args) -> void
{
  throw_diagnostic<TypeError>(std::forward<Args>(t_args)...);
}
} // namespace diagnostic

#endif // ACRIS_ACRIS_DIAGNOSTIC_TYPE_ERROR_HPP
