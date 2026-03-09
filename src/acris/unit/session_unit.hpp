#ifndef SESSION_UNIT_HPP
#define SESSION_UNIT_HPP

// STL Includes:
#include <map>
#include <memory>
#include <set>
#include <string>

namespace unit {
// Forward Declarations:
class SessionUnit;

namespace settings {
struct Settings;
}

// Aliases:
using SessionUnitPtr = std::shared_ptr<SessionUnit>;

using MacroDefs = std::map<std::string, std::string>;
using MacroUndefs = std::set<std::string>;

// Classes:
/*!
 * Stores the configuration for each @ref TranslationUnit.
 * This includes the instantiated backends.
 * And which build dir should be used.
 */
class SessionUnit {
  private:
  MacroDefs m_mdefs;
  bool m_no_libc;

  public:
  SessionUnit(MacroDefs t_mdefs, bool t_no_libc);

  auto macro_defs() const -> const MacroDefs&;
  auto no_libc() const -> bool;

  virtual ~SessionUnit() = default;
};

// Functions:
template<typename... Args>
inline auto make_session_unit(Args&&... t_args) -> SessionUnitPtr
{
  using unit::SessionUnit;

  return std::make_shared<SessionUnit>(std::forward<Args>(t_args)...);
}
} // namespace unit

#endif // SESSION_UNIT_HPP
