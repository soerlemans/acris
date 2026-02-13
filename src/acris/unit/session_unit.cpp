#include "session_unit.hpp"

namespace unit {
SessionUnit::SessionUnit(MacroDefs t_mdefs): m_mdefs{std::move(t_mdefs)}
{}

auto SessionUnit::macro_defs() const -> const MacroDefs&
{
  return m_mdefs;
}

auto make_session_unit(const Settings& t_settings) -> SessionUnitPtr
{
  return make_shared<unit::SessionUnit>(t_settings.m_mdefs);
}
} // namespace unit
