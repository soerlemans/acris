#include "type_promoter.hpp"

// STL Includes:
#include <sstream>

// Absolute Includes:
#include "acris/debug/log.hpp"
#include "acris/diagnostic/diagnostic_error.hpp"

// Internal:
namespace {
//! Reserve an arbitrary amount of entries as an optimization.
constexpr auto prealloc_map_size{10};
} // namespace

namespace semantic {
// Methods:
auto TypePromoter::get_priority(const TypeLadder& t_ladder,
                                const NativeType t_type) const
  -> TypeLadder::mapped_type
{
  using diagnostic::throw_diagnostic;

  TypeLadder::mapped_type priority{0};

  const auto iter{t_ladder.find(t_type)};
  if(iter != t_ladder.end()) {
    priority = iter->second;
  } else {
    std::stringstream ss;

    ss << "NativeType does not exist in floating point TypeLadder.\n";
    ss << "Did not find " << t_type << '\n';

    throw_diagnostic(ss.str());
  }

  return priority;
}

TypePromoter::TypePromoter()
  : m_float{prealloc_map_size}, m_int{prealloc_map_size}
{
  // Type precedence ladders.

  // Float:
  m_float.insert({NativeType::F32, 0});
  m_float.insert({NativeType::F64, 1});

  // Integer:
  m_int.insert({NativeType::INT, 0});
  m_int.insert({NativeType::UINT, 1});
  m_int.insert({NativeType::I8, 2});
  m_int.insert({NativeType::U8, 3});
  m_int.insert({NativeType::I16, 4});
  m_int.insert({NativeType::U16, 5});
  m_int.insert({NativeType::I32, 6});
  m_int.insert({NativeType::U32, 7});
  m_int.insert({NativeType::I64, 8});
  m_int.insert({NativeType::U64, 9});
  m_int.insert({NativeType::ISIZE, 10});
  m_int.insert({NativeType::USIZE, 11});
}

auto TypePromoter::promote_bool(const NativeType t_lhs) const -> NativeTypeOpt
{
  NativeTypeOpt opt{};

  const auto integer{m_int.contains(t_lhs)};

  // Only integers may be promoted to boolean's (implicitely).
  if(integer) {
    opt = NativeType::BOOL;
  }

  return opt;
}

auto TypePromoter::promote_float(const NativeType t_lhs, const NativeType t_rhs,
                                 const PromotionMode t_mode) const
  -> NativeTypeOpt
{
  NativeTypeOpt opt{};

  // TODO: Handle Int and UInt to float promotion.
  const auto lhs_priority{get_priority(m_float, t_lhs)};
  const auto rhs_priority{get_priority(m_float, t_rhs)};

  if(t_mode == PromotionMode::PROMOTE_TO_LHS) {
    if(lhs_priority >= rhs_priority) {
      opt = t_lhs;
    } else {
      // TODO: error.
    }
  } else if(t_mode == PromotionMode::ENFORCE_RHS) {
    opt = t_rhs;
  } else if(t_mode == PromotionMode::PEAK) {
    opt = (lhs_priority > rhs_priority) ? t_lhs : t_rhs;
  }

  return opt;
}

auto TypePromoter::promote_int(const NativeType t_lhs, const NativeType t_rhs,
                               const PromotionMode t_mode) const
  -> NativeTypeOpt
{
  NativeTypeOpt opt{};

  const auto lhs_priority{get_priority(m_int, t_lhs)};
  const auto rhs_priority{get_priority(m_int, t_rhs)};

  // First check if RHS is castable to LHS.
  if(t_mode == PromotionMode::PROMOTE_TO_LHS) {
    if(lhs_priority >= rhs_priority) {
      opt = t_lhs;
    } else {
      // TODO: error.
    }
  } else if(t_mode == PromotionMode::ENFORCE_RHS) {
    opt = t_rhs;
  } else if(t_mode == PromotionMode::PEAK) {
    opt = (lhs_priority > rhs_priority) ? t_lhs : t_rhs;
  }

  return opt;
}

auto TypePromoter::promote(const NativeType t_lhs, const NativeType t_rhs,
                           const PromotionMode t_mode) const -> NativeTypeOpt
{
  NativeTypeOpt opt{};

  const auto in_ladder{[&](const TypeLadder& t_ladder) {
    const auto lhs{t_ladder.contains(t_lhs)};
    const auto rhs{t_ladder.contains(t_rhs)};

    return lhs && rhs;
  }};

  // if(m_float.contains(t_lhs) || m_float.contains(t_rhs)) {
  //   opt = promote_float(t_lhs, t_rhs);
  // } else

  if(in_ladder(m_int)) {
    opt = promote_int(t_lhs, t_rhs, t_mode);
  }

  return opt;
}
} // namespace semantic
