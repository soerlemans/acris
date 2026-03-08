#include "pratt_parser.hpp"

// STL Includes:
#include <memory>
#include <stdexcept>

// Absolute Includes:
#include "acris/ast/node/include_nodes.hpp"
#include "acris/debug/log.hpp"

namespace parser::pratt {
// Using Statements:
NODE_USING_ALL_NAMESPACES()

// Methods:
PrattParser::PrattParser(ParserContextPtr t_ctx,
                         PrattParserDelegate* t_delegate)
  : Parser{t_ctx}, m_delegate{t_delegate}, m_prefix{}, m_infix{}
{}

// Prefix parsing:
auto PrattParser::prefix_expr(const TokenType t_type) -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);

  const auto [lbp, rbp] = m_prefix.at(t_type);

  return expr(rbp);
}

auto PrattParser::lvalue() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto token{get_token()};

  if(auto ptr{m_delegate->scope_resolution()}; ptr) {
    node = std::move(ptr);
  } else if(next_if(TokenType::IDENTIFIER)) {
    const auto id{token.str()};
    const auto pos{token.position()};
    DBG_TRACE_PRINT(INFO, "Found 'IDENTIFIER': ", id);
    node = make_node<IdentifierNode>(pos, id);
  }

  return node;
}

//! Prefix operator parses prefix increment and decrement
auto PrattParser::literal() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto token{get_token()};
  if(next_if(TokenType::FLOAT_LITERAL)) {
    const auto val{token.float_()};
    PARSER_FOUND(TokenType::FLOAT_LITERAL, " literal: ", val);
    node = make_node<Float>(val);

  } else if(next_if(TokenType::INTEGER_LITERAL)) {
    const auto val{token.int_()};
    PARSER_FOUND(TokenType::INTEGER_LITERAL, " literal: ", val);
    node = make_node<Integer>(val);

  } else if(next_if(TokenType::CHAR_LITERAL)) {
    const auto val{token.char_()};
    PARSER_FOUND(TokenType::CHAR_LITERAL, " literal: ", val);
    node = make_node<Char>(val);

  } else if(next_if(TokenType::STRING_LITERAL)) {
    const auto val{token.str()};
    PARSER_FOUND(TokenType::STRING_LITERAL, " literal: ", val);
    node = make_node<String>(val);

  } else if(next_if(TokenType::TRUE)) {
    PARSER_FOUND(TokenType::TRUE);
    node = make_node<Boolean>(true);

  } else if(next_if(TokenType::FALSE)) {
    PARSER_FOUND(TokenType::FALSE);
    node = make_node<Boolean>(false);

  } else if(check(TokenType::BRACKET_OPEN)) { // Note uses check().
    DBG_TRACE_PRINT(VERBOSE, "Found 'ARRAY EXPR'");

    auto list{brackets([this] {
      return m_delegate->expr_list_opt();
    })};

    node = make_node<ArrayExpr>(list);
  } else if(check(TokenType::IOTA)) {
    m_delegate->context_check_enum();

    throw_syntax_error("TODO: Implement iota.");
  }

  return node;
}

auto PrattParser::grouping() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(check(TokenType::PAREN_OPEN)) {
    DBG_TRACE_PRINT(VERBOSE, "Found 'GROUPING'");

    node = parens([this] {
      return this->expr();
    });
  }

  return node;
}

auto PrattParser::negation() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto pos{get_position()};
  if(next_if(TokenType::NOT)) {
    PARSER_FOUND(TokenType::NOT);

    if(auto ptr{prefix_expr(TokenType::NOT)}; ptr) {
      node = make_node<Not>(pos, std::move(ptr));
    } else {
      throw_syntax_error("After a negation (!) an expression must follow");
    }
  }

  return node;
}

auto PrattParser::address_of() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto token{get_token()};
  if(next_if(TokenType::AMPERSAND)) {
    DBG_TRACE_PRINT(VERBOSE, "Found 'ADDRESS OF'");

    NodePtr rhs{prefix_expr(token.type())};
    if(!rhs) {
      throw_syntax_error("Expected an expression after &.");
    }

    node = make_node<AddressOf>(std::move(rhs));
  }

  return node;
}

auto PrattParser::dereference() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto token{get_token()};
  if(next_if(TokenType::ASTERISK)) {
    DBG_TRACE_PRINT(VERBOSE, "Found 'DEREFERENCE'");

    NodePtr rhs{prefix_expr(token.type())};
    if(!rhs) {
      throw_syntax_error("Expected an expression after *.");
    }

    node = make_node<Dereference>(std::move(rhs));
  }

  return node;
}

//! Parses unary prefixes like having a + or - before an expression
auto PrattParser::unary_prefix() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto token{get_token()};
  if(next_if(TokenType::PLUS, TokenType::MINUS)) {
    DBG_TRACE_PRINT(VERBOSE, "Found 'UNARY PREFIX'");

    NodePtr rhs{prefix_expr(token.type())};
    if(!rhs) {
      throw_syntax_error("Expected an expression after + or -.");
    }

    node = make_node<UnaryPrefix>(token.type(), std::move(rhs));
  }

  return node;
}

//! This function parses function calls
auto PrattParser::function_call() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto token{get_token()};

  if(next_if(TokenType::IDENTIFIER)) {
    // Dont prematurely confuse variables and function calls
    if(!check(TokenType::PAREN_OPEN)) {
      prev();

      return node;
    }

    auto args{parens([this] {
      return m_delegate->expr_list_opt();
    })};

    const auto id{token.str()};
    DBG_TRACE_PRINT(INFO, "Found a 'FUNCTION CALL': ", id);

    node = make_node<FunctionCall>(id, std::move(args));
  }

  return node;
}

auto PrattParser::member_access() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto token{get_token()};
  if(next_if(TokenType::IDENTIFIER)) {
    const auto id{token.str()};
    const auto pos{token.position()};

    DBG_TRACE_PRINT(INFO, "Found 'MEMBER': ", id);
    node = make_node<Member>(pos, id);
  }

  return node;
}

auto PrattParser::method_call() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto token{get_token()};

  if(next_if(TokenType::IDENTIFIER)) {
    // Dont prematurely confuse variables and function calls
    if(!check(TokenType::PAREN_OPEN)) {
      prev();

      return node;
    }

    auto args{parens([this] {
      return m_delegate->expr_list_opt();
    })};

    const auto id{token.str()};
    DBG_TRACE_PRINT(INFO, "Found a 'METHOD CALL': ", id);

    node = make_node<MethodCall>(id, std::move(args));
  }

  return node;
}

auto PrattParser::prefix() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(auto ptr{m_delegate->self()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{grouping()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{address_of()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{dereference()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{unary_prefix()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{negation()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{literal()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{function_call()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{lvalue()}; ptr) {
    node = std::move(ptr);
  }

  return node;
}

auto PrattParser::prefix_chainable() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(auto ptr{function_call()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{member_access()}; ptr) {
    node = std::move(ptr);
  }

  return node;
}

auto PrattParser::prefix_chain() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(auto ptr{method_call()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{member_access()}; ptr) {
    node = std::move(ptr);
  }

  return node;
}

// Infix parsing:
auto PrattParser::infix_chain(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  // Guard clause:
  if(t_lhs == nullptr) {
    return nullptr;
  }

  // TODO: Eliminate as many dynamic_cast calls as possible.
  // Chain expressions can only be performed on the following:
  const auto* is_variable{dynamic_cast<IdentifierNode*>(t_lhs.get())};
  const auto* is_function_call{dynamic_cast<FunctionCall*>(t_lhs.get())};
  const auto* is_method_call{dynamic_cast<MethodCall*>(t_lhs.get())};
  const auto* is_member_access{dynamic_cast<MemberAccess*>(t_lhs.get())};
  const auto* is_member{dynamic_cast<Member*>(t_lhs.get())};

  // Guard clause:
  // Only allow chain infix on following.
  if(!is_variable && !is_function_call && !is_method_call && !is_member_access
     && !is_member) {
    return node;
  }

  const auto pos{get_position()};
  if(after_newlines(TokenType::DOT)) {
    PARSER_FOUND(TokenType::DOT);
    const auto token{get_token()};
    next();

    if(auto rhs{t_fn(token.type())}; rhs) {
      node = make_node<MemberAccess>(pos, std::move(t_lhs), std::move(rhs));
    }

    // }else if(after_newlines(TokenType::BRACKET_OPEN)) {
    // } else if(after_newlines(TokenType::PAREN_OPEN)) {
    // TODO: Add for '[]' and '()'.
  }

  return node;
}

auto PrattParser::arithmetic(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto pos{get_position()};
  const auto lambda{[&](ArithmeticOp t_op) {
    const auto token{get_token()};
    next();

    if(auto rhs{t_fn(token.type())}; rhs) {
      node = make_node<Arithmetic>(pos, t_op, std::move(t_lhs), std::move(rhs));
    }
  }};

  if(after_newlines(TokenType::ASTERISK)) {
    PARSER_FOUND(TokenType::ASTERISK);
    lambda(ArithmeticOp::MULTIPLY);

  } else if(after_newlines(TokenType::SLASH)) {
    PARSER_FOUND(TokenType::SLASH);
    lambda(ArithmeticOp::DIVIDE);

  } else if(after_newlines(TokenType::PERCENT_SIGN)) {
    PARSER_FOUND(TokenType::PERCENT_SIGN);
    lambda(ArithmeticOp::MODULO);

  } else if(after_newlines(TokenType::PLUS)) {
    PARSER_FOUND(TokenType::PLUS);
    lambda(ArithmeticOp::ADD);

  } else if(after_newlines(TokenType::MINUS)) {
    PARSER_FOUND(TokenType::MINUS);
    lambda(ArithmeticOp::SUBTRACT);
  }

  return node;
}

auto PrattParser::logical(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto pos{get_position()};
  auto lambda{[&]<typename T>() {
    const auto token{get_token()};
    next();

    if(auto rhs{t_fn(token.type())}; rhs) {
      node = make_node<T>(pos, std::move(t_lhs), std::move(rhs));
    }
  }};

  if(after_newlines(TokenType::AND)) {
    PARSER_FOUND(TokenType::AND);
    lambda.template operator()<And>();

  } else if(after_newlines(TokenType::OR)) {
    PARSER_FOUND(TokenType::OR);
    lambda.template operator()<Or>();
  }

  return node;
}

auto PrattParser::comparison(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(t_lhs) {
    const auto pos{get_position()};
    const auto lambda{[&](const ComparisonOp t_op) {
      const auto token{get_token()};
      next();

      if(auto rhs{t_fn(token.type())}; rhs) {
        node =
          make_node<Comparison>(pos, t_op, std::move(t_lhs), std::move(rhs));
      }
    }};

    if(after_newlines(TokenType::LESS_THAN)) {
      PARSER_FOUND(TokenType::LESS_THAN);
      lambda(ComparisonOp::LESS_THAN);

    } else if(after_newlines(TokenType::LESS_THAN_EQUAL)) {
      PARSER_FOUND(TokenType::LESS_THAN_EQUAL);
      lambda(ComparisonOp::LESS_THAN_EQUAL);

    } else if(after_newlines(TokenType::EQUAL)) {
      PARSER_FOUND(TokenType::EQUAL);
      lambda(ComparisonOp::EQUAL);

    } else if(after_newlines(TokenType::NOT_EQUAL)) {
      PARSER_FOUND(TokenType::NOT_EQUAL);
      lambda(ComparisonOp::NOT_EQUAL);

    } else if(after_newlines(TokenType::GREATER_THAN)) {
      PARSER_FOUND(TokenType::GREATER_THAN);
      lambda(ComparisonOp::GREATER_THAN);

    } else if(after_newlines(TokenType::GREATER_THAN_EQUAL)) {
      PARSER_FOUND(TokenType::GREATER_THAN_EQUAL);
      lambda(ComparisonOp::GREATER_THAN_EQUAL);
    }
  }

  return node;
}

auto PrattParser::infix(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  // Guard clause:
  if(t_lhs == nullptr) {
    return nullptr;
  }

  if(auto ptr{arithmetic(t_lhs, t_fn)}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{logical(t_lhs, t_fn)}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{comparison(t_lhs, t_fn)}; ptr) {
    node = std::move(ptr);
  }

  return node;
}

auto PrattParser::member_access(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto pos{get_position()};

  if(after_newlines(TokenType::DOT)) {
    PARSER_FOUND(TokenType::DOT);
    const auto token{get_token()};
    next();

    if(auto rhs{t_fn(token.type())}; rhs) {
      node = make_node<MemberAccess>(pos, std::move(t_lhs), std::move(rhs));
    } else {
      // TODO: Maybe error?
    }
  }

  return node;
}

auto PrattParser::subscript(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto pos{get_position()};

  if(after_newlines(TokenType::BRACKET_OPEN)) {
    PARSER_FOUND(TokenType::BRACKET_OPEN);
    const auto token{get_token()};
    next();

    if(auto rhs{t_fn(token.type())}; rhs) {
      node = make_node<Subscript>(pos, std::move(t_lhs), std::move(rhs));
      expect(TokenType::BRACKET_CLOSE);

    } else {
      // TODO: Maybe error?
    }
  }

  return node;
}

auto PrattParser::call([[maybe_unused]] NodePtr& t_lhs,
                       [[maybe_unused]] const RhsFn& t_fn) -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto pos{get_position()};

  /* TODO: Implement call node.
  if(after_newlines(TokenType::PAREN_OPEN)) {
    PARSER_FOUND(TokenType::PAREN_OPEN);
    const auto token{get_token()};
    next();

    if(auto rhs{t_fn(token.type())}; rhs) {
      node = make_node<Call>(pos, std::move(t_lhs), std::move(rhs));
      expect(TokenType::PAREN_CLOSE);

    } else {
      prev();
    }
  }
  */

  return node;
}

auto PrattParser::to_cast(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto pos{get_position()};
  if(after_newlines(TokenType::TO)) {
    PARSER_FOUND(TokenType::TO);
    const auto token{get_token()};
    next();

    expect(TokenType::LESS_THAN);

    if(auto rhs{t_fn(token.type())}; rhs) {
      node = make_node<ToCast>(pos, std::move(t_lhs));
      expect(TokenType::GREATER_THAN);

    } else {
      prev();
      prev();
    }
  }

  return node;
}

auto PrattParser::postfix(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(t_lhs == nullptr) {
    return nullptr;
  }

  if(auto ptr{member_access(t_lhs, t_fn)}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{subscript(t_lhs, t_fn)}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{call(t_lhs, t_fn)}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{to_cast(t_lhs, t_fn)}; ptr) {
    node = std::move(ptr);
  }

  return node;
}

// Expressions:
auto PrattParser::chain_expr(const int t_min_bp) -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr lhs{prefix_chain()};

  // Infix:
  while(!eos()) {
    const auto rhs_chain_fn{[&](const TokenType t_type) {
      NodePtr rhs{};

      const auto [lbp, rbp] = m_infix.at(t_type);
      if(lbp < t_min_bp) {
        prev();
      } else {
        rhs = chain_expr(rbp);
        if(!rhs) {
          throw_syntax_error(
            "Chain infix operations require a right hand side");
        }
      }

      return rhs;
    }};

    if(auto ptr{infix_chain(lhs, rhs_chain_fn)}; ptr) {
      lhs = std::move(ptr);
    } else {
      break;
    }
  }

  return lhs;
}

auto PrattParser::expr(const int t_min_bp) -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr lhs{prefix()};

  // Infix:
  while(!eos()) {
    // Postfix parsing.
    const auto rhs_postfix_fn{[&](const TokenType t_type) {
      NodePtr rhs{};

      const auto [lbp, rbp] = m_postfix.at(t_type);
      if(lbp < t_min_bp) {
        prev();
      } else {
        rhs = expr(rbp);
        if(!rhs) {
          throw_syntax_error("Postfix operations require a right hand side");
        }
      }

      return rhs;
    }};

    // Check for postfix to the prefix.
    if(auto ptr{postfix(lhs, rhs_postfix_fn)}; ptr) {
      lhs = std::move(ptr);
    }

    // Infix parsing.
    const auto rhs_infix_fn{[&](const TokenType t_type) {
      NodePtr rhs{};

      const auto [lbp, rbp] = m_infix.at(t_type);
      if(lbp < t_min_bp) {
        prev();
      } else {
        rhs = expr(rbp);
        if(!rhs) {
          throw_syntax_error("Infix operations require a right hand side");
        }
      }

      return rhs;
    }};

    // If we do not find the expression quit.
    if(auto ptr{infix(lhs, rhs_infix_fn)}; ptr) {
      lhs = std::move(ptr);
    } else {
      break;
    }
  }

  return lhs;
}

// Lvalue specific:
auto PrattParser::lvalue_chain(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto pos{get_position()};
  if(after_newlines(TokenType::DOT)) {
    PARSER_FOUND(TokenType::DOT);
    const auto token{get_token()};
    next();

    if(auto rhs{t_fn(token.type())}; rhs) {
      node = make_node<MemberAccess>(pos, std::move(t_lhs), std::move(rhs));
    }

  } else if(after_newlines(TokenType::BRACKET_OPEN)) {
    PARSER_FOUND(TokenType::BRACKET_OPEN);
    const auto token{get_token()};
    next();

    const auto array_subscript{expr()};
    expect(TokenType::BRACKET_CLOSE);
    // if(auto rhs{t_fn(token.type())}; rhs) {
    //   node = make_node<ArrayAccess>(pos, std::move(t_lhs), std::move(rhs));
    // }

    // TODO: Figure out how to work this thing.
  }
  // } else if(after_newlines(TokenType::PAREN_OPEN)) {}
  // TODO: Add for '[]' and '()'.

  return node;
}

auto PrattParser::lvalue_infix(NodePtr& t_lhs, const RhsFn& t_fn) -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(t_lhs) {
    if(auto ptr{lvalue_chain(t_lhs, t_fn)}; ptr) {
      node = std::move(ptr);
    }
  }

  return node;
}

// auto PrattParser::lvalue_member_expr(const int t_min_bp) -> NodePtr
// {
//   DBG_TRACE_FN(VERBOSE);
//   NodePtr lhs{member_access()};

//   // Infix:
//   while(!eos()) {
//     const auto rhs_fn{[&](const TokenType t_type) {
//       NodePtr rhs{};

//       const auto [lbp, rbp] = m_infix.at(t_type);
//       if(lbp < t_min_bp) {
//         prev();
//       } else {
//         rhs = lvalue_member_expr(rbp);
//         if(!rhs) {
//           throw_syntax_error(
//             "Lvalue member infix operations require a right hand side");
//         }
//       }

//       return rhs;
//     }};

//     // If we do not find the expression quit.
//     if(auto ptr{lvalue_infix(lhs, rhs_fn)}; ptr) {
//       lhs = std::move(ptr);
//     } else {
//       break;
//     }
//   }

//   return lhs;
// }

// auto PrattParser::lvalue_expr(const int t_min_bp) -> NodePtr
// {
//   DBG_TRACE_FN(VERBOSE);
//   NodePtr lhs{lvalue()};

//   if(!lhs) {
//     // FIXME: Makes self.self.self a valid expression.
//     lhs = m_delegate->self();
//   }

//   // Infix:
//   while(!eos()) {
//     const auto rhs_fn{[&](const TokenType t_type) {
//       NodePtr rhs{};

//       const auto [lbp, rbp] = m_infix.at(t_type);
//       if(lbp < t_min_bp) {
//         prev();
//       } else {
//         rhs = lvalue_member_expr(rbp);
//         if(!rhs) {
//           throw_syntax_error(
//             "Lvalue infix operations require a right hand side");
//         }
//       }

//       return rhs;
//     }};

//     // If we do not find the expression quit.
//     if(auto ptr{lvalue_infix(lhs, rhs_fn)}; ptr) {
//       lhs = std::move(ptr);
//     } else {
//       break;
//     }
//   }

//   return lhs;
// }

auto PrattParser::method_call_expr(int t_min_bp) -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr lhs{prefix_chainable()};

  if(!lhs) {
    // FIXME: Makes self.self.self a valid expression.
    lhs = m_delegate->self();
  }

  // TODO: Somehow confirm that this is a method call at the end of the chain.

  // Infix:
  while(!eos()) {
    const auto rhs_fn{[&](const TokenType t_type) {
      NodePtr rhs{};

      const auto [lbp, rbp] = m_infix.at(t_type);
      if(lbp < t_min_bp) {
        prev();
      } else {
        rhs = chain_expr(rbp);
        if(!rhs) {
          throw_syntax_error(
            "Lvalue infix operations require a right hand side");
        }
      }

      return rhs;
    }};

    // If we do not find the expression quit.
    if(auto ptr{infix_chain(lhs, rhs_fn)}; ptr) {
      lhs = std::move(ptr);
    } else {
      break;
    }
  }

  return lhs;
}
} // namespace parser::pratt
