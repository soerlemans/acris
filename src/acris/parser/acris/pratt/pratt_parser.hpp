#ifndef ACRIS_ACRIS_PARSER_ACRIS_PRATT_PRATT_PARSER_HPP
#define ACRIS_ACRIS_PARSER_ACRIS_PRATT_PRATT_PARSER_HPP

// STL Includes:
#include <memory>
#include <unordered_map>
#include <utility>

// Absolute Includes:
#include "acris/parser/parser.hpp"
#include "lib/stdtypes.hpp"

// Local Includes:
#include "binding/maps.hpp"

namespace parser::pratt {
// Using Statements:
using binding::InfixMap;
using binding::PostfixMap;
using binding::PrefixMap;

using ParseFn = std::function<NodePtr(int)>;
//! This type is used to get the right hand side of a binary expressions
using RhsFn = std::function<NodePtr(TokenType, ParseFn)>;

struct RhsContext {
  RhsFn m_rhs;
  ParseFn m_parse;

  auto operator()(const TokenType t_type) const -> NodePtr
  {
    // Use default parsing target.
    return m_rhs(t_type, m_parse);
  }

  auto operator()(const TokenType t_type, ParseFn&& t_fn) const -> NodePtr
  {
    // If we need custom behavior, like parsing a type spec.
    return m_rhs(t_type, t_fn);
  }
};

// Structs:
struct PrattParserDelegate {
  PrattParserDelegate() = default;

  virtual auto type_expr() -> NodePtr = 0;
  virtual auto scope_resolution() -> NodePtr = 0;

  // Needed to check if we can use iota.
  virtual auto context_check_enum() -> void = 0;
  virtual auto expr_list_opt() -> NodeListPtr = 0;
  virtual auto self() -> NodePtr = 0;

  virtual ~PrattParserDelegate() = default;
};

// Classes:
class PrattParser : public Parser {
  private:
  PrattParserDelegate* m_delegate;

  // Note these come from the binding submodule.
  PrefixMap m_prefix;
  InfixMap m_infix;
  PostfixMap m_postfix;

  public:
  explicit PrattParser(ParserContextPtr t_ctx, PrattParserDelegate* t_delegate);

  // TODO: The chain expression stuff needs to be cleaned.
  // Up and gain some clarity.

  // Grammar:
  // Prefix parsing:
  virtual auto prefix_expr(TokenType t_type) -> NodePtr;
  virtual auto lvalue() -> NodePtr;
  virtual auto literal() -> NodePtr;
  virtual auto grouping() -> NodePtr;
  virtual auto address_of() -> NodePtr;
  virtual auto dereference() -> NodePtr;
  virtual auto unary_prefix() -> NodePtr;
  virtual auto negation() -> NodePtr;
  virtual auto function_call() -> NodePtr;
  virtual auto member_access() -> NodePtr;
  virtual auto method_call() -> NodePtr;

  virtual auto prefix() -> NodePtr;

  /*!
   * Prefixes/nodes that can be chained.
   */
  virtual auto prefix_chainable() -> NodePtr;

  /*!
   * Prefixes/nodes we match when we are actually in a chain.
   */
  virtual auto prefix_chain() -> NodePtr;

  // Infix parsing:
  virtual auto infix_chain(NodePtr& t_lhs, const RhsContext& t_ctx) -> NodePtr;

  virtual auto arithmetic(NodePtr& t_lhs, const RhsContext& t_ctx) -> NodePtr;
  virtual auto logical(NodePtr& t_lhs, const RhsContext& t_ctx) -> NodePtr;
  virtual auto comparison(NodePtr& t_lhs, const RhsContext& t_ctx) -> NodePtr;

  virtual auto infix(NodePtr& t_lhs, const RhsContext& t_ctx) -> NodePtr;

  // Postfix parsing:
  virtual auto member_access(NodePtr& t_lhs, const RhsContext& t_ctx)
    -> NodePtr;
  virtual auto subscript(NodePtr& t_lhs, const RhsContext& t_ctx) -> NodePtr;
  virtual auto call(NodePtr& t_lhs, const RhsContext& t_ctx) -> NodePtr;
  virtual auto to_cast(NodePtr& t_lhs, const RhsContext& t_ctx) -> NodePtr;

  virtual auto postfix(NodePtr& t_lhs, const RhsContext& t_ctx) -> NodePtr;

  // Expressions:
  //! Continues a chain expression not an entry.
  virtual auto chain_expr(int t_min_bp = 0) -> NodePtr;

  //! Entry for normal expressions.
  virtual auto expr(int t_min_bp = 0) -> NodePtr;

  // Side effect escape hatch.
  /*!
   * Escape hatch for statements which produce side effects.
   * Usually function calls/method calls.
   */
  virtual auto effect_expr(int t_min_bp = 0) -> NodePtr;

  virtual ~PrattParser() = default;
};
} // namespace parser::pratt

#endif // ACRIS_ACRIS_PARSER_ACRIS_PRATT_PRATT_PARSER_HPP
