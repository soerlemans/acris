#ifndef ACRIS_ACRIS_PARSER_ACRIS_ACRIS_PARSER_HPP
#define ACRIS_ACRIS_PARSER_ACRIS_ACRIS_PARSER_HPP

// STL Includes:
#include <string>

// Relative Includes:
#include "pratt/pratt_parser.hpp"
#include "type/type_parser.hpp"

// Local Includes:
#include "context/context_guard.hpp"

namespace parser::acris {
// Using Statements:
using ast::node::packaging::Import;
using context::Context;
using context::ContextStore;
using pratt::PrattParser;
using pratt::PrattParserDelegate;
using type::TypeParser;

// Aliases:
using EvalPair = std::pair<NodePtr, NodePtr>;

template<typename T>
using AccessorFn = std::function<NodePtr(T&)>;

// Classes:
/*!
 * Top down parser of the Acris language.
 * See acris.yy for grammar specification (BNF).
 */
class AcrisParser : public Parser, public PrattParserDelegate {
  private:
  PrattParser m_pratt;
  TypeParser m_type;

  ContextStore m_store;

  protected:
  /*!
   * Check if a certain parsing context is enabled.
   *
   * @note If the passed Context is not enabled throws a @ref SyntaxError.
   *
   * @param[in] t_context Context to check for.
   */
  auto context_check(Context t_context) -> void;
  auto context_check_enum() -> void;

  public:
  AcrisParser(TokenStream t_token_stream);
  AcrisParser(ParserContextPtr t_ctx);

  // Grammar:
  virtual auto newline_opt() -> void;
  virtual auto terminator() -> void;

  // Needed for cross access in type and pratt parser.
  virtual auto type_expr() -> NodePtr;
  virtual auto expr() -> NodePtr;

  // Expressions:
  virtual auto literal_list() -> NodeListPtr;
  virtual auto expr_opt() -> NodePtr;

  virtual auto init_expr(TokenType t_type) -> NodePtr;
  virtual auto let_expr() -> NodePtr;
  virtual auto var_expr() -> NodePtr;
  virtual auto binding_expr() -> NodePtr;
  virtual auto eval_expr() -> EvalPair;

  virtual auto expr_statement() -> NodePtr;

  // Expression lists:
  virtual auto expr_list() -> NodeListPtr;
  virtual auto expr_list_opt() -> NodeListPtr;

  // Result statement:
  virtual auto assignment() -> NodePtr;
  virtual auto result_statement() -> NodePtr;

  // Jump statements:
  virtual auto jump_statement() -> NodePtr;

  // Loop statements:
  virtual auto loop_statement() -> NodePtr;

  // Switch statements:
  virtual auto case_clause() -> NodePtr;
  virtual auto case_body() -> NodeListPtr;

  virtual auto switch_case() -> NodePtr;
  virtual auto switch_else() -> NodePtr;
  virtual auto switch_fallthrough() -> NodePtr;
  virtual auto switch_statement() -> NodePtr;

  // Branch statements:
  virtual auto branch_statement(TokenType t_type) -> NodePtr;
  virtual auto if_statement() -> NodePtr;
  virtual auto elif_statement() -> NodePtr;

  // Statements:
  virtual auto statement() -> NodePtr;
  virtual auto statement_list() -> NodeListPtr;

  // Body:
  virtual auto body() -> NodeListPtr;

  // User Types:
  virtual auto scope_resolution() -> NodePtr;

  // Enum my beloved:
  virtual auto enum_field() -> NodePtr;
  virtual auto enum_field_list() -> NodeListPtr;
  virtual auto enum_def() -> NodePtr;

  // Struct:
  virtual auto member_decl() -> NodePtr;
  virtual auto member_decl_list() -> NodeListPtr;
  virtual auto struct_def() -> NodePtr;

  virtual auto type_def() -> NodePtr;

  virtual auto self() -> NodePtr;

  // Function:
  virtual auto param_list() -> NodeListPtr;
  virtual auto param_list_opt() -> NodeListPtr;

  virtual auto return_type() -> NodePtr;
  virtual auto return_type_opt() -> NodePtr;

  virtual auto lambda() -> NodePtr;
  virtual auto function() -> NodePtr;

  virtual auto attribute_item() -> NodePtr;
  virtual auto attribute_item_list() -> NodeListPtr;
  virtual auto attribute_body() -> NodeListPtr;
  virtual auto attribute() -> NodePtr;

  virtual auto declare() -> NodePtr;

  // Import:
  virtual auto import_expr(Import& t_import) -> bool;
  virtual auto import_list(Import& t_import) -> void;
  virtual auto import_() -> NodePtr;

  // Module:
  virtual auto module_decl() -> NodePtr;

  // Items:
  virtual auto item() -> NodePtr;
  virtual auto item_list() -> NodeListPtr;

  virtual auto program() -> NodeListPtr;

  //! Produces AST.
  virtual auto parse() -> NodePtr;

  // Used to expose pratt and type parsing.
  // As the parsers are cyclicly dependent on eachother.
  virtual auto pratt_parse(AccessorFn<PrattParser> t_fn) -> NodePtr;
  virtual auto type_parse(AccessorFn<TypeParser> t_fn) -> NodePtr;

  virtual ~AcrisParser() = default;
};

} // namespace parser::acris

#endif // ACRIS_ACRIS_PARSER_ACRIS_ACRIS_PARSER_HPP
