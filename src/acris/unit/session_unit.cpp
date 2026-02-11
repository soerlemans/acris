#include "session_unit.hpp"

namespace unit {
SessionUnit::SessionUnit(Settings&& t_settings)
  : m_settings{std::move(t_settings)}
{}

auto SessionUnit::macro_definitions() const -> const MacroDefinitions&
{
  return m_macro_definitions;
}
} // namespace unit
