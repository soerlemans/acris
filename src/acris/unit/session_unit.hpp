#ifndef SESSION_UNIT_HPP
#define SESSION_UNIT_HPP

// Absolute Includes:
#include <settings/settings.hpp>

namespace unit {
// Forward Declarations:
class SessionUnit;

// Aliases:
using settings::MacroDefinitionsMap;
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
  MacroDefinitionsMap m_macro_definitions;

  public:
  SessionUnit(Settings&& t_settings);

  auto macro_defintions() const -> const MacroDefinitionsMap&;

  virtual ~SessionUnit() = default;
};

// Functions:
//! Resolve CLI settings and configuration from project.toml.
auto make_build_unit(BuildUnitParams& t_params) -> BuildUnitPtr;


} // namespace unit

#endif // SESSION_UNIT_HPP
