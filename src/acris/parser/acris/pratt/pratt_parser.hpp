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

//! This type is used to get the right hand side of a binary expressions
using RhsFn = std::function<NodePtr(TokenType)>;

// Structs:
struct PrattParserDelegate {
  PrattParserDelegate() = default;

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
  virtual auto infix_chain(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr;

  virtual auto arithmetic(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr;
  virtual auto logical(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr;
  virtual auto comparison(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr;

  virtual auto infix(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr;

  // Postfix parsing:
  virtual auto member_access(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr;
  virtual auto subscript(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr;
  virtual auto call(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr;

  virtual auto postfix(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr;

  // Expressions:
  //! Continues a chain expression not an entry.
  virtual auto chain_expr(int t_min_bp = 0) -> NodePtr;

  //! Entry for normal expressions.
  virtual auto expr(int t_min_bp = 0) -> NodePtr;

  // Lvalue specific:
  virtual auto lvalue_chain(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr;
  virtual auto lvalue_infix(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr;

  /*!
   * Continue's a member access chain.
   * Which derives from an lvalue_expr().
   * Not an entry.
   */
  virtual auto lvalue_member_expr(int t_min_bp = 0) -> NodePtr;

  /*!
   * Generally an expression is universal.
   * But when we are dealing with precedence as the destination for a value.
   * We only allow assignable statements on the left hand side.
   * So no assigning to the result of function calls.
   * Entry for Lvalue expressions.
   */
  virtual auto lvalue_expr(int t_min_bp = 0) -> NodePtr;

  // Method call specific.
  /*!
   * The only free standing chain expr which we do not assign to.
   * Is a method_call at the end of a chain expr.
   * This is an entry for Method Call expressions.
   */
  virtual auto method_call_expr(int t_min_bp = 0) -> NodePtr;

  virtual ~PrattParser() = default;
};
} // namespace parser::pratt

#endif // ACRIS_ACRIS_PARSER_ACRIS_PRATT_PRATT_PARSER_HPP
