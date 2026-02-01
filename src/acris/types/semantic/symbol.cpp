#include "symbol.hpp"

// STL Includes:
#include <iostream>

// Absolute Includes:
#include "acris/debug/log.hpp"

// Local Includes:
#include "symbol_types.hpp"


// Using Statements:
using namespace types::symbol;
using namespace std::literals::string_view_literals;

// Functions:
auto operator<<(std::ostream& t_os, StructTypePtr t_struct) -> std::ostream&
{
  // TODO: Redo these operator<< functions they are outdated.

  if(t_struct) {
    // t_os << "Identifier: " << t_struct->m_identifier;
    // t_os << ", Members: {";

    t_os << "(struct ";
    t_os << t_struct->m_identifier;

    std::string_view sep{""};
    for(const auto& [id, type_data] : t_struct->m_members) {
      t_os << sep << "(" << id << ", " << type_data << ")";

      sep = ", ";
    }

    t_os << "})";

  } else {
    DBG_ERROR("(StructTypePtr) nullptr!");

    t_os << "nullptr";
  }

  return t_os;
}

auto operator<<(std::ostream& t_os, FnTypePtr t_fn) -> std::ostream&
{
  if(t_fn) {
    auto sep{""sv};
    t_os << "(";
    for(const auto& param : t_fn->m_params) {
      t_os << sep << param;

      sep = ", ";
    }
    t_os << ") -> " << t_fn->m_return_type;
  } else {
    DBG_ERROR("(FnTypePtr) nullptr!");

    t_os << "nullptr";
  }

  return t_os;
}

auto operator<<(std::ostream& t_os, PointerTypePtr t_ptr) -> std::ostream&
{
  if(t_ptr) {
    for(int index{0}; index < t_ptr->m_indirection; index++) {
      t_os << '*';
    }

    if(t_ptr->m_readonly) {
      t_os << "ro";
    }

    t_os << t_ptr->m_type;
  } else {
    DBG_ERROR("(PointerTypePtr) nullptr!");

    t_os << "nullptr";
  }

  return t_os;
}

auto operator<<(std::ostream& t_os, ArrayTypePtr t_arr) -> std::ostream&
{
  if(t_arr) {
    t_os << "[" << t_arr->m_type << "; " << t_arr->m_size << "]";
  } else {
    DBG_ERROR("(ArrayTypePtr) nullptr!");

    t_os << "nullptr";
  }

  return t_os;
}

auto operator<<(std::ostream& t_os, VarTypePtr t_var) -> std::ostream&
{
  if(t_var) {
    t_os << "(var ";
    if(t_var->is_mutable()) {
      t_os << "mutable ";
    }

    t_os << t_var->m_type;
    t_os << ")";
  } else {
    DBG_ERROR("(VarTypePtr) nullptr!");

    t_os << "nullptr";
  }

  return t_os;
}

auto operator<<(std::ostream& t_os, const SymbolDataList& t_list)
  -> std::ostream&
{
  using namespace std::literals::string_view_literals;

  auto sep{""sv};
  for(const auto& elem : t_list) {
    t_os << sep << elem;
    sep = ", ";
  }

  return t_os;
}
