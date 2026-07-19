#ifndef ACRIS_ACRIS_DIAGNOSTIC_DIAGNOSTIC_ERROR_HPP
#define ACRIS_ACRIS_DIAGNOSTIC_DIAGNOSTIC_ERROR_HPP

// STL Includes:
#include <format>
#include <sstream>
#include <string>

namespace diagnostic {
// Classes:
/*!
 * Base diagnostic error class.
 * Includes a stacktrace when compiling.
 * As long as the @ref NDEBUG macro is not defined.
 * Used to signal when a user has made a mistake.
 */
class DiagnosticError {
  protected:
  std::string m_error;

  public:
  DiagnosticError(std::string_view t_msg);

  virtual auto what() const noexcept -> const std::string_view;

  // TODO: Maybe keep the stack trace separately accessible?
  // auto trace() const noexcept -> const stacktrace&;

  virtual ~DiagnosticError() = default;
};

// Functions:
template<typename T = DiagnosticError, typename... Args>
  requires std::is_base_of<DiagnosticError, T>::value
inline auto throw_diagnostic(Args&&... t_args) -> void
{
  std::stringstream ss;

  // Add all arguments to the string stream.
  (ss << ... << t_args);

  throw T{ss.view()};
}

template<typename T = DiagnosticError, typename... Args>
  requires std::is_base_of<DiagnosticError, T>::value
inline auto throwf_diagnostic(std::format_string<Args...> fmt, Args&&... t_args)
  -> void
{
  std::stringstream ss;

  // Add all arguments to the string stream.
  (ss << ... << t_args);

  ss << std::format(fmt, std::forward<Args>(t_args)...);

  throw T{ss.view()};
}
} // namespace diagnostic

#endif // ACRIS_ACRIS_DIAGNOSTIC_DIAGNOSTIC_ERROR_HPP
