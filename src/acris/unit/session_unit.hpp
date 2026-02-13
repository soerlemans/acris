#ifndef SESSION_UNIT_HPP
#define SESSION_UNIT_HPP

// STL Includes:
#include <memory>

// Absolute Includes:
#include "acris/settings/settings.hpp"

namespace unit {
// Forward Declarations:
class SessionUnit;

// Aliases:
using SessionUnitPtr = std::shared_ptr<SessionUnit>;

using settings::MacroDefs;
using settings::Settings;

using SessionUnitPtr = std::shared_ptr<SessionUnit>;

// Classes:
/*!
 * Stores the configuration for each @ref TranslationUnit.
 * This includes the instantiated backends.
 * And which build dir should be used.
 */
class SessionUnit {
  private:
  MacroDefs m_mdefs;

  public:
  SessionUnit(MacroDefs t_mdefs);

  auto macro_defs() const -> const MacroDefs&;

  virtual ~SessionUnit() = default;
};

// Functions:
auto make_session_unit(const Settings& t_settings)
  -> SessionUnitPtr;
} // namespace unit

#endif // SESSION_UNIT_HPP
