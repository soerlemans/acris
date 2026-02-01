#ifndef ACRIS_ACRIS_PARSER_PARSER_HPP
#define ACRIS_ACRIS_PARSER_PARSER_HPP

// STL Includes:
#include <functional>
#include <memory>

// Absolute Includes:
#include "acris/ast/node/fdecl.hpp"
#include "acris/debug/trace.hpp"
#include "acris/token/token_stream.hpp"
#include "acris/token/tokentype2str.hpp"

// Local Includes:
#include "backtrack_guard.hpp"


// Macros:
#define PARSER_FOUND(t_tokentype, ...)                    \
  do {                                                    \
    const auto str{tokentype2str(t_tokentype)};           \
    const auto quoted{std::quoted(str, '\'')};            \
    DBG_TRACE_PRINT(INFO, "Found ", quoted, __VA_ARGS__); \
  } while(false)

namespace parser {
// Forward Decl:
struct ParserContext;

// Using Statements:
using ast::node::NodeListPtr;
using ast::node::NodePtr;

using container::TextPosition;

using token::Token;
using token::TokenStream;
using token::TokenType;
using token::tokentype2str;

// Aliases:
using ParseFn = std::function<ast::node::NodePtr()>;
using ParserContextPtr = std::shared_ptr<ParserContext>;

struct ParserContext {
  TokenStream m_token_stream;
  int m_counter;
};

// Classes:
/*!
 * Abstract parser class provides utilities that a parser would need to
 * implement
 */
class Parser {
  private:
  ParserContextPtr m_ctx;

  protected:
  DBG_TRACE_INIT_EXPR(m_ctx->m_counter)

  auto get_token_stream() -> TokenStream&;

	// TODO: Someday use std::allocate_shared with an arena allocator.
  /*!
   * Wrapper method for std::make_shared() makes it easy to change smart
   * pointer type later down the line
   */
  template<typename T, typename... Args>
  inline auto make_node(Args&&... t_args) -> std::shared_ptr<T>
  {
    return std::make_shared<T>(std::forward<Args>(t_args)...);
  }

  // Helper methods for parsing:
  auto throw_syntax_error(std::string_view t_msg) const -> void;
  auto throw_if_eos(std::string_view t_msg) const -> void;

  auto eos() const -> bool;

  auto check(TokenType t_type) -> bool;
  auto next() -> Token&;
  auto expect(TokenType t_type) -> Token&;
  auto prev() -> Token&;
  auto get_token() const -> Token&;

  auto get_position() const -> const TextPosition&;

  template<typename... Args>
  auto next_if(Args&&... t_args) -> bool
  {
    const auto is_next{(check(std::forward<Args>(t_args)) || ...)};

    if(is_next) {
      next();
    }

    return is_next;
  }

  auto after_newlines(TokenType t_type) -> bool;

  // TODO: create .tpp for these definitions?
  template<typename Fn>
  inline auto surround(const TokenType t_open, const TokenType t_close,
                       const Fn t_fn)
  {
    DBG_TRACE_FN(VERBOSE);

    expect(t_open);
    auto var{t_fn()};
    expect(t_close);

    return var;
  }

  template<typename Fn>
  inline auto parens(const Fn t_fn)
  {
    DBG_TRACE_FN(VERBOSE);

    return surround(TokenType::PAREN_OPEN, TokenType::PAREN_CLOSE, t_fn);
  }

  template<typename Fn>
  inline auto accolades(const Fn t_fn)
  {
    return surround(TokenType::ACCOLADE_OPEN, TokenType::ACCOLADE_CLOSE, t_fn);
  }

  
  template<typename Fn>
  inline auto brackets(const Fn t_fn)
  {
    DBG_TRACE_FN(VERBOSE);

    return surround(TokenType::BRACKET_OPEN, TokenType::BRACKET_CLOSE, t_fn);
  }

  auto list_of(ParseFn t_fn) -> ast::node::NodeListPtr;

  auto context() -> ParserContextPtr;

  public:
  Parser(ParserContextPtr t_ctx);

  virtual ~Parser() = default;
};

// Functions:
[[nodiscard("Pure function must use result.")]]
auto make_parser_context(TokenStream&& t_token_stream) -> ParserContextPtr;
} // namespace parser

#endif // ACRIS_ACRIS_PARSER_PARSER_HPP
