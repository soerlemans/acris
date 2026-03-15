#ifndef ACRIS_ACRIS_SEMANTIC_SEMANTIC_VALIDATOR_HPP
#define ACRIS_ACRIS_SEMANTIC_SEMANTIC_VALIDATOR_HPP

// STL Includes:
#include <optional>

// Absolute Includes:
#include "acris/container/text_position.hpp"
#include "acris/types/semantic/symbol.hpp"

// Local Includes:
#include "symbol_env_state.hpp"
#include "type_promoter.hpp"

namespace semantic {
// Using:
using container::TextPosition;
using types::symbol::SymbolData;

// Aliases:
using SymbolDataOpt = std::optional<SymbolData>;

// Enums:
// enum class BindingExprType {
// 	TYPE
// };

// Structs:
struct BindingExprData {
  SymbolDataOpt m_init_expr;
  std::string m_identifier;
  SymbolDataOpt m_type;
  TextPosition m_position;
};

struct UnaryOperationData {
  SymbolData m_lhs;
  TextPosition m_position;
};

struct BinaryOperationData {
  SymbolData m_lhs;
  SymbolData m_rhs;
  TextPosition m_position;
};

// Classes:
/*!
 * This class is responsible.
 * For cleanly validating the semantic validity of various expressions.
 * Its meant for a cleaner separation of concerns from @ref SemanticChecker.
 */
class SemanticValidator {
  private:
  TypePromoter m_type_promoter;

  protected:
  // auto error();

  public:
  SemanticValidator() = default;

  auto handle_condition(const SymbolData& t_data,
                        const TextPosition& t_pos) const -> void;

  //! Handle type promotion between two different types.
  auto promote(const SymbolData& t_lhs, const SymbolData& rhs,
               PromotionMode t_mode = PromotionMode::PROMOTE_TO_LHS) const
    -> NativeTypeOpt;


	/* TODO: Implement.
  auto register_struct() -> SymbolData;
  auto register_alias() -> SymbolData;

  auto validate_function() -> SymbolData;
  auto validate_method() -> SymbolData;

  auto validate_call() -> SymbolData;
	*/

  auto validate_binding_expr(const BindingExprData& t_data) -> SymbolData;

  auto validate_unary_prefix() -> SymbolData;

  //! Validate logical `not`.
  auto validate_logical_unop(const UnaryOperationData& t_data) -> SymbolData;

  //! Validate logical `and` and `or`.
  auto validate_logical_binop(const BinaryOperationData& t_data) -> SymbolData;

  auto validate_arithmetic(const BinaryOperationData& t_data) -> SymbolData;
  auto validate_assignment(const BinaryOperationData& t_data) -> SymbolData;
  auto validate_comparison(const BinaryOperationData& t_data) -> SymbolData;

  virtual ~SemanticValidator() = default;
};
} // namespace semantic

#endif // ACRIS_ACRIS_SEMANTIC_SEMANTIC_VALIDATOR_HPP
