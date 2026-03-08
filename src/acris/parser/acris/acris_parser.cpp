#include "acris_parser.hpp"

// STL Includes:
#include <exception>
#include <format>
#include <string>

// Absolute Includes:
#include "acris/ast/node/include_nodes.hpp"
#include "acris/debug/trace.hpp"

// Macros:
#define CONTEXT_GUARD(t_context)                   \
  context::ContextGuard CONCAT(trace, __COUNTER__) \
  {                                                \
    context::Context::t_context, m_store           \
  }

namespace parser::acris {
// Using statements:
NODE_USING_ALL_NAMESPACES()

// Protected Methods:
auto AcrisParser::context_check(const Context t_context) -> void
{
  const auto enabled{m_store.get(t_context)};

  if(!enabled) {
    const auto msg{std::format("Keyword used outside of proper context ({}).",
                               context2str(t_context))};

    throw_syntax_error(msg);
  }
}

auto AcrisParser::context_check_enum() -> void
{
  context_check(Context::ENUM);
}

// Public Methods:
AcrisParser::AcrisParser(TokenStream t_token_stream)
  : AcrisParser{make_parser_context(std::move(t_token_stream))}
{}

AcrisParser::AcrisParser(ParserContextPtr t_ctx)
  : Parser{t_ctx},
    m_pratt{t_ctx, (PrattParserDelegate*)this},
    m_type{t_ctx},
    m_store{}
{}

auto AcrisParser::newline_opt() -> void
{
  DBG_TRACE_FN(VERBOSE);

  while(!eos() && next_if(TokenType::NEWLINE)) {
    DBG_TRACE_PRINT(INFO, "Found 'NEWLINE'");
  }
}

auto AcrisParser::terminator() -> void
{
  DBG_TRACE_FN(VERBOSE);

  const auto is_terminator{[this] {
    return next_if(TokenType::SEMICOLON, TokenType::NEWLINE);
  }};

  if(!is_terminator()) {
    throw_syntax_error("Expected at least one 'TERMINATOR'");
  }

  while(!eos()) {
    if(is_terminator()) {
      DBG_TRACE_PRINT(INFO, "Found 'TERMINATOR'");
    } else {
      break;
    }
  }
}

auto AcrisParser::type_expr() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);

  return m_type.type_expr();
}

auto AcrisParser::expr() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);

  return m_pratt.expr();
}

auto AcrisParser::literal_list() -> NodeListPtr
{
  DBG_TRACE_FN(VERBOSE);
  auto nodes{make_node<List>()};

  auto pos{get_position()};
  if(auto first_ptr{m_pratt.literal()}; first_ptr) {
    nodes->push_back(std::move(first_ptr));

    while(!eos()) {
      if(next_if(TokenType::COMMA)) {
        auto ptr{m_pratt.literal()};
        if(!ptr) {
          throw_syntax_error("Expected another literal!");
        }

        nodes->push_back(std::move(ptr));
      } else {
        break;
      }
    }
  }

  if(nodes->empty()) {
    throw_syntax_error("Expected a list of literals");
  }

  return nodes;
}

auto AcrisParser::expr_opt() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);

  return m_pratt.expr();
}

// FIXME: This is "temporary method", improve later.
auto AcrisParser::init_expr(const TokenType t_init_type) -> NodePtr
{
  using token::TokenType;

  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto pos{get_position()};
  if(next_if(t_init_type)) {
    PARSER_FOUND(t_init_type);

    const auto id{expect(TokenType::IDENTIFIER)};
    NodePtr type{};
    NodePtr expr_ptr{};

    const auto get_expr{[&]() {
      newline_opt();
      expr_ptr = m_pratt.expr();
    }};

    if(next_if(TokenType::COLON)) {
      type = m_type.type_expr();

      if(next_if(TokenType::ASSIGNMENT)) {
        newline_opt();
        expr_ptr = m_pratt.expr();
      } else if(t_init_type == TokenType::LET) {
        throw_syntax_error("Variables declared with Let must be initialized.");
      }
    } else {
      expect(TokenType::ASSIGNMENT);
      newline_opt();
      expr_ptr = m_pratt.expr();
    }

    // TODO: Have this selection be done more elegantly.
    if(t_init_type == TokenType::VAR) {
      node =
        make_node<Var>(pos, id.str(), std::move(type), std::move(expr_ptr));
    } else if(t_init_type == TokenType::LET) {
      node =
        make_node<Let>(pos, id.str(), std::move(type), std::move(expr_ptr));
    } else {
      // TODO: Error
    }
  }

  return node;
}

auto AcrisParser::let_expr() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);

  return init_expr(TokenType::LET);
}

auto AcrisParser::var_expr() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);

  return init_expr(TokenType::VAR);
}

auto AcrisParser::binding_expr() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(auto ptr{var_expr()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{let_expr()}; ptr) {
    node = std::move(ptr);
  }

  return node;
}

auto AcrisParser::eval_expr() -> EvalPair
{
  DBG_TRACE_FN(VERBOSE);
  EvalPair pair{};

  if(auto ptr{binding_expr()}; ptr) {
    expect(TokenType::SEMICOLON);
    auto cond_expr{m_pratt.expr()};

    pair = {std::move(ptr), cond_expr};
  } else if(auto ptr{m_pratt.expr()}; ptr) {
    pair.second = std::move(ptr);
  }

  return pair;
}

auto AcrisParser::expr_statement() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(auto ptr{m_pratt.expr()}; ptr) {
    terminator();
    node = std::move(ptr);
  }

  return node;
}

auto AcrisParser::expr_list() -> NodeListPtr
{
  DBG_TRACE_FN(VERBOSE);
  auto nodes{expr_list_opt()};

  if(nodes->empty()) {
    throw_syntax_error("Expected a list of expressions");
  }

  return nodes;
}

auto AcrisParser::expr_list_opt() -> NodeListPtr
{
  DBG_TRACE_FN(VERBOSE);
  auto nodes{make_node<List>()};

  if(auto ptr{m_pratt.expr()}; ptr) {
    DBG_TRACE_PRINT(INFO, "Found 'EXPR");

    nodes->push_back(std::move(ptr));
  }

  while(!eos()) {
    if(next_if(TokenType::COMMA)) {
      newline_opt();
      if(auto ptr{m_pratt.expr()}; ptr) {
        DBG_TRACE_PRINT(INFO, "Found ',' 'EXPR'");

        nodes->push_back(std::move(ptr));
      } else {
        throw_syntax_error("Expected another expression after ','");
      }
    } else {
      break;
    }
  }

  return nodes;
}

// Result statement:
auto AcrisParser::assignment() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  PARSER_BACKTRACK_GUARD(node);

  if(auto lhs{m_pratt.expr()}; lhs) {
    const auto pos{get_position()};
    const auto lambda{[&](const AssignmentOp t_op) {
      newline_opt();
      if(auto rhs{m_pratt.expr()}; rhs) {
        node = make_node<Assignment>(pos, t_op, std::move(lhs), std::move(rhs));
      }
    }};

    if(next_if(TokenType::MUL_ASSIGN)) {
      PARSER_FOUND(TokenType::MUL_ASSIGN);
      lambda(AssignmentOp::MULTIPLY);

    } else if(next_if(TokenType::DIV_ASSIGN)) {
      PARSER_FOUND(TokenType::DIV_ASSIGN);
      lambda(AssignmentOp::DIVIDE);

    } else if(next_if(TokenType::MOD_ASSIGN)) {
      PARSER_FOUND(TokenType::MOD_ASSIGN);
      lambda(AssignmentOp::MODULO);

    } else if(next_if(TokenType::ADD_ASSIGN)) {
      PARSER_FOUND(TokenType::ADD_ASSIGN);
      lambda(AssignmentOp::ADD);

    } else if(next_if(TokenType::SUB_ASSIGN)) {
      PARSER_FOUND(TokenType::SUB_ASSIGN);
      lambda(AssignmentOp::SUBTRACT);

    } else if(next_if(TokenType::ASSIGNMENT)) {
      PARSER_FOUND(TokenType::ASSIGNMENT);
      lambda(AssignmentOp::REGULAR);

      // Postcrement part of the method.
    } else if(next_if(TokenType::INCREMENT)) {
      PARSER_FOUND(TokenType::INCREMENT);
      node = make_node<Increment>(std::move(lhs));
    } else if(next_if(TokenType::DECREMENT)) {
      PARSER_FOUND(TokenType::DECREMENT);
      node = make_node<Decrement>(std::move(lhs));
    }
  }

  return node;
}

auto AcrisParser::result_statement() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(auto ptr{m_pratt.function_call()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{binding_expr()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{assignment()}; ptr) {
    node = std::move(ptr);
    // Breaks stuff now.
    // } else if(auto ptr{m_pratt.effect_expr()}; ptr) {
    //   node = std::move(ptr);
  }

  // Terminate result statement.
  if(node) {
    terminator();
  }

  return node;
}

auto AcrisParser::jump_statement() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(check(TokenType::CONTINUE)) {
    context_check(Context::LOOP);
    next();

    terminator();
    node = make_node<Continue>();
  } else if(check(TokenType::BREAK)) {
    context_check(Context::LOOP);
    next();

    terminator();
    node = make_node<Break>();
  } else if(next_if(TokenType::DEFER)) {
    auto body_ptr{make_node<List>()};
    if(auto ptr{body()}; ptr) {
      body_ptr = std::move(ptr);
    } else if(auto ptr{expr_statement()}; ptr) {
      body_ptr->push_back(std::move(ptr));
    }

    node = make_node<Defer>(std::move(body_ptr));
  } else if(next_if(TokenType::RETURN)) {
    auto expr_ptr{expr_opt()};
    terminator();

    node = make_node<Return>(std::move(expr_ptr));
  }

  return node;
}

// FIXME: Currently i++ (postcrement) is not a expression.
// So we have to change the grammar to allow result_statements or similar.
// Potentially we could add a new grammar rule special for loops.
auto AcrisParser::loop_statement() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto pos{get_position()};
  if(next_if(TokenType::LOOP)) {
    auto [init, cond] = eval_expr();

    NodePtr expr_ptr;
    if(next_if(TokenType::SEMICOLON)) {
      expr_ptr = assignment();
    }

    // We are in the loop context now.
    CONTEXT_GUARD(LOOP);
    auto body_ptr{body()};
    node = make_node<Loop>(pos, std::move(init), std::move(cond),
                           std::move(expr_ptr), std::move(body_ptr));
  }

  return node;
}

auto AcrisParser::branch_statement(const TokenType t_type) -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto pos{get_position()};
  if(next_if(t_type)) {
    auto [init, cond] = eval_expr();
    if(!cond) {
      throw_syntax_error("Expected a conditional");
    }

    auto then_ptr{body()};

    NodeListPtr alt_ptr{};
    if(next_if(TokenType::ELSE)) {
      alt_ptr = body();
    } else if(auto ptr{branch_statement(TokenType::ELIF)}; ptr) {
      // Elifs are structured in the AST as just an if in an else branch
      alt_ptr = make_node<List>();
      alt_ptr->push_back(std::move(ptr));
    }

    node = make_node<If>(pos, std::move(init), std::move(cond),
                         std::move(then_ptr), std::move(alt_ptr));
  }

  return node;
}

auto AcrisParser::if_statement() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  return branch_statement(TokenType::IF);
}

auto AcrisParser::elif_statement() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);

  return branch_statement(TokenType::ELIF);
}

// TODO: Error out on literals in the body those should be illegal.
auto AcrisParser::statement() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(auto ptr{binding_expr()}; ptr) {
    terminator();
    node = std::move(ptr);
  } else if(auto ptr{result_statement()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{if_statement()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{loop_statement()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{jump_statement()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{body()}; ptr) {
    node = std::move(ptr);
    // TODO: Detect expressions which have no side effects and throw.
    // } else if() {
    // throw_syntax_error();
  }

  return node;
}

auto AcrisParser::statement_list() -> NodeListPtr
{
  DBG_TRACE_FN(VERBOSE);

  return list_of([this] {
    NodePtr stmnt{statement()};

    if(stmnt) {
      newline_opt();
    }

    return stmnt;
  });
}

// Body:
/*!
 * The @ref body() should never be optional.
 * Or else we will consume newlines unintentionally.
 */
auto AcrisParser::body() -> NodeListPtr
{
  DBG_TRACE_FN(VERBOSE);
  NodeListPtr nodes{};

  // After a list of newlines an accolade most occur
  if(after_newlines(TokenType::ACCOLADE_OPEN)) {
    expect(TokenType::ACCOLADE_OPEN);

    newline_opt();
    if(auto ptr{statement_list()}; ptr) {
      nodes = std::move(ptr);
    }

    expect(TokenType::ACCOLADE_CLOSE);
  }

  return nodes;
}

auto AcrisParser::scope_resolution() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  // TODO: add recursive path resolution later.
  const auto token{get_token()};
  if(next_if(TokenType::IDENTIFIER)) {
    const auto id{token.str()};
    const auto pos{token.position()};

    if(next_if(TokenType::SCOPE_RESOLUTION)) {
      DBG_TRACE_PRINT(INFO, "Found 'SCOPE_RESOLUTION': ", id);

      ScopeResolutionPath path{};
      path.push_back(id);

      // Qualified name target.
      auto target_expr{m_pratt.lvalue()};

      node = make_node<ScopeResolution>(pos, std::move(path),
                                        std::move(target_expr));
    } else {
      // Not scope resolution undo peek-forward.
      prev();
    }
  }

  return node;
}

// Enum:
auto AcrisParser::enum_field() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto token{get_token()};
  if(next_if(TokenType::IDENTIFIER)) {
    const auto id{token.str()};

    NodePtr expr{};
    if(next_if(TokenType::ASSIGNMENT)) {

      expr = m_pratt.expr();
    }

    node = make_node<EnumField>(id, std::move(expr));
  }

  return node;
}

auto AcrisParser::enum_field_list() -> NodeListPtr
{
  DBG_TRACE_FN(VERBOSE);

  return list_of([this] {
    auto node{enum_field()};

    if(next_if(TokenType::COMMA)) {
      newline_opt(); // Remove newlines.
    } else {
      if(check(TokenType::ACCOLADE_CLOSE)) {
        return node;
      } else {
        // If Comma is not present and we dont find end of enum body.
        // We should throw as the enum is malformed.
        throw_syntax_error("Expected a comma for next enum field.");
      }
    }

    return node;
  });
}

auto AcrisParser::enum_def() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(next_if(TokenType::ENUM)) {
    PARSER_FOUND(TokenType::ENUM);

    const auto id{expect(TokenType::IDENTIFIER).str()};
    newline_opt();

    // If type defined.
    NodePtr type{};
    if(next_if(TokenType::COLON)) {
      type = m_type.type_expr();
    }

    // Needed to enable iota.
    CONTEXT_GUARD(ENUM);
    auto fields{accolades([this] {
      newline_opt();

      return enum_field_list();
    })};

    node = make_node<Enum>(id, std::move(type), std::move(fields));
  }

  return node;
}

// Struct:
auto AcrisParser::member_decl() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto token{get_token()};
  if(next_if(TokenType::IDENTIFIER)) {
    const auto id{token.str()};
    expect(TokenType::COLON);

    auto type{m_type.type_expr()};
    terminator();

    node = make_node<MemberDecl>(id, std::move(type));
  }

  return node;
}

auto AcrisParser::member_decl_list() -> NodeListPtr
{
  DBG_TRACE_FN(VERBOSE);

  return list_of([this] {
    return member_decl();
  });
}

auto AcrisParser::struct_def() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(next_if(TokenType::STRUCT)) {
    PARSER_FOUND(TokenType::STRUCT);

    const auto id{expect(TokenType::IDENTIFIER).str()};
    newline_opt();

    auto members{accolades([this] {
      newline_opt();

      return member_decl_list();
    })};

    node = make_node<Struct>(id, std::move(members));
  }

  return node;
}

auto AcrisParser::type_def() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(auto ptr{struct_def()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{enum_def()}; ptr) {
    node = std::move(ptr);
  }

  return node;
}

auto AcrisParser::self() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto token{get_token()};
  if(next_if(TokenType::SELF)) {
    context_check(Context::METHOD);

    DBG_TRACE_PRINT(INFO, "Found 'SELF'.");
    node = make_node<Self>();
  }

  return node;
}

// Function:
auto AcrisParser::param_list() -> NodeListPtr
{
  DBG_TRACE_FN(VERBOSE);
  auto nodes{param_list_opt()};

  if(nodes->empty()) {
    throw_syntax_error("Expected list of parameters");
  }

  return nodes;
}

auto AcrisParser::param_list_opt() -> NodeListPtr
{
  DBG_TRACE_FN(VERBOSE);
  auto nodes{make_node<List>()};

  auto pos{get_position()};
  if(check(TokenType::IDENTIFIER)) {
    PARSER_FOUND(TokenType::IDENTIFIER);

    const auto id{expect(TokenType::IDENTIFIER).str()};
    expect(TokenType::COLON);
    auto type{m_type.type_expr()};

    nodes->push_back(make_node<Parameter>(pos, id, std::move(type)));

    while(!eos()) {
      if(next_if(TokenType::COMMA)) {
        pos = get_position();
        const auto id{expect(TokenType::IDENTIFIER).str()};
        PARSER_FOUND(TokenType::COMMA, "'IDENTIFIER'");

        expect(TokenType::COLON);
        auto type{m_type.type_expr()};

        nodes->push_back(make_node<Parameter>(pos, id, std::move(type)));
      } else {
        break;
      }
    }
  }

  return nodes;
}

// TODO: Create a StrOpt type
auto AcrisParser::return_type() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  const auto node{return_type_opt()};

  if(node == nullptr) {
    throw_syntax_error("Expected a return type!");
  }

  return node;
}

auto AcrisParser::return_type_opt() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(after_newlines(TokenType::ARROW)) {
    expect(TokenType::ARROW);

    node = m_type.type_expr();
    if(node == nullptr) {
      prev();
    }
  }

  return node;
}

auto AcrisParser::lambda() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(next_if(TokenType::FUNCTION)) {
    auto params{parens([this] {
      return this->param_list_opt();
    })};
  }

  return node;
}

auto AcrisParser::function() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(next_if(TokenType::FUNCTION)) {
    PARSER_FOUND(TokenType::FUNCTION);

    // NodeListPtr params;
    NodeListPtr params{};

    // TODO: Split method and function handling in their own parts.

    // Handle method:
    if(next_if(TokenType::PAREN_OPEN)) {
      auto receiver_type{expect(TokenType::IDENTIFIER).str()};
      expect(TokenType::PAREN_CLOSE);

      const auto tt_id{expect(TokenType::IDENTIFIER)};
      const auto id{tt_id.str()};

      expect(TokenType::PAREN_OPEN);
      params = param_list_opt();
      expect(TokenType::PAREN_CLOSE);

      auto return_type{return_type_opt()};

      // We are in the method context now.
      CONTEXT_GUARD(METHOD);
      auto body_ptr{body()};
      if(!body_ptr) {
        throw_syntax_error("Expected a method body");
      }

      node = make_node<Method>(id, receiver_type, std::move(params),
                               std::move(return_type), std::move(body_ptr));
      // Handle function:
    } else {
      const auto tt_id{expect(TokenType::IDENTIFIER)};
      const auto id{tt_id.str()};

      expect(TokenType::PAREN_OPEN);

      // Dealing with a function:
      params = param_list_opt();
      expect(TokenType::PAREN_CLOSE);

      auto return_type{return_type_opt()};
      auto body_ptr{body()};
      if(!body_ptr) {
        throw_syntax_error("Expected a function body");
      }

      node = make_node<Function>(id, std::move(params), std::move(return_type),
                                 std::move(body_ptr));
    }
  }

  return node;
}

auto AcrisParser::attribute_item() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(auto ptr{attribute()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{declare()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{binding_expr()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{function()}; ptr) {
    node = std::move(ptr);
  }

  return node;
}

auto AcrisParser::attribute_item_list() -> NodeListPtr
{
  DBG_TRACE_FN(VERBOSE);

  return list_of([this] {
    NodePtr attr_item{attribute_item()};

    if(attr_item) {
      newline_opt();
    }

    return attr_item;
  });
}

auto AcrisParser::attribute_body() -> NodeListPtr
{
  DBG_TRACE_FN(VERBOSE);
  auto nodes{make_node<List>()};

  if(next_if(TokenType::ACCOLADE_OPEN)) {
    PARSER_FOUND(TokenType::ACCOLADE_OPEN);
    newline_opt();
    nodes = attribute_item_list();
    newline_opt();
    expect(TokenType::ACCOLADE_CLOSE);
  } else if(auto ptr{attribute_item()}; ptr) {
    nodes->push_back(std::move(ptr));
    newline_opt();
  } else {
    throw_syntax_error("Expected an attribute item or body.");
  }

  return nodes;
}

auto AcrisParser::attribute() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(next_if(TokenType::ATTRIBUTE_OPEN)) {
    PARSER_FOUND(TokenType::ATTRIBUTE_OPEN);
    const auto id{expect(TokenType::IDENTIFIER).str()};
    NodeListPtr params_ptr{};
    NodeListPtr body_ptr{make_node<List>()};

    // Get any potential parameters (can be nullptr or zero).
    if(next_if(TokenType::PAREN_OPEN)) {
      params_ptr = literal_list();

      expect(TokenType::PAREN_CLOSE);
    }
    expect(TokenType::ATTRIBUTE_CLOSE);

    // Get the item or body.
    newline_opt();
    body_ptr = attribute_body();

    node = make_node<Attribute>(id, std::move(params_ptr), std::move(body_ptr));
  }

  return node;
}

auto AcrisParser::declare() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(next_if(TokenType::DECLARE)) {
    PARSER_FOUND(TokenType::DECLARE);
    std::string id{};
    NodePtr type{};
    NodePtr ret_type{};

    if(next_if(TokenType::FUNCTION)) {
      id = expect(TokenType::IDENTIFIER).str();

      auto params{parens([this] {
        return this->param_list_opt();
      })};

      ret_type = return_type();

      node =
        make_node<FunctionDecl>(id, std::move(params), std::move(ret_type));
    } else if(next_if(TokenType::LET)) {
      id = expect(TokenType::IDENTIFIER).str();
      expect(TokenType::COLON);
      type = m_type.type_expr();

      node = make_node<LetDecl>(id, std::move(type));
    } else if(next_if(TokenType::VAR)) {
      id = expect(TokenType::IDENTIFIER).str();
      expect(TokenType::COLON);
      type = m_type.type_expr();

      node = make_node<VarDecl>(id, std::move(type));
    } else {
      // TODO: Error handle.
    }
  }

  return node;
}

auto AcrisParser::import_expr(Import& t_import) -> bool
{
  DBG_TRACE_FN(VERBOSE);
  bool is_import_expr{true};

  const auto token{get_token()};
  if(next_if(TokenType::STRING_LITERAL)) {
    auto str{token.str()};
    DBG_TRACE_PRINT(INFO, "Found: ", std::quoted(str));

    t_import.add_import(std::move(str));
  } else if(next_if(TokenType::IDENTIFIER)) {
    auto id{token.str()};

    expect(TokenType::ASSIGNMENT);
    auto str{expect(TokenType::STRING_LITERAL).str()};

    DBG_TRACE_PRINT(INFO, "Found alias: ", id, " = ", std::quoted(str));

    t_import.add_import({str, id});
  } else {
    is_import_expr = false;
  }

  return is_import_expr;
}

auto AcrisParser::import_list(Import& t_import) -> void
{
  DBG_TRACE_FN(VERBOSE);

  if(!import_expr(t_import)) {
    throw_syntax_error("Expected at least one import expression!");
  }

  while(!eos()) {
    if(check(TokenType::SEMICOLON)) {
      terminator();

      if(!import_expr(t_import)) {
        throw_syntax_error("Expected another import expression!");
      }
    } else if(next_if(TokenType::NEWLINE)) {
      newline_opt();

      if(!import_expr(t_import)) {
        break;
      }
    } else {
      break;
    }
  }
}

auto AcrisParser::import_() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(next_if(TokenType::IMPORT)) {
    PARSER_FOUND(TokenType::IMPORT);

    auto import_ptr{make_node<Import>()};
    if(check(TokenType::STRING_LITERAL)) {
      PARSER_FOUND(TokenType::STRING_LITERAL);
      const auto id{get_token()};
      next();

      import_ptr->add_import(id.str());
    } else if(next_if(TokenType::PAREN_OPEN)) {
      newline_opt();
      import_list(*import_ptr);
      expect(TokenType::PAREN_CLOSE);
    } else {
      throw_syntax_error("Expected string_LITERAL or an import list");
    }

    node = std::move(import_ptr);
  }

  return node;
}

auto AcrisParser::module_decl() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(next_if(TokenType::MODULE)) {
    PARSER_FOUND(TokenType::MODULE);
    const auto id{expect(TokenType::IDENTIFIER)};

    node = make_node<ModuleDecl>(id.str());
  }

  return node;
}

auto AcrisParser::item() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(auto ptr{import_()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{declare()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{attribute()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{type_def()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{binding_expr()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{function()}; ptr) {
    node = std::move(ptr);
  }

  return node;
}

// Item list exists out of an item followed by a terminator
// Till there are are no more items
auto AcrisParser::item_list() -> NodeListPtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  // Check for the
  if(auto ptr{module_decl()}; ptr) {
    node = std::move(ptr);
  } else {
    throw_syntax_error("First statement in acrislang should be a module name.");
  }

  // Parsing of item_list.
  const auto parse_lambda{[this] -> NodePtr {
    // Remove newlines before item.
    newline_opt();
    if(eos()) {
      return nullptr;
    }

    return item();
  }};

  // Get the item list.
  auto list{list_of(parse_lambda)};
  if(node) {
    // Prepend the module declaration.
    list->push_front(std::move(node));
  }

  return list;
}

// This is the grammars root node.
auto AcrisParser::program() -> NodeListPtr
{
  DBG_TRACE_FN(VERBOSE);

  return item_list();
}

auto AcrisParser::parse() -> NodePtr
{
  const auto ast{program()};

  // TODO: Check if we are at EOS.
  // If we are not at EOS, then we didnt parse everything.
  // Which is a parsing error or syntax error.
  // As the entirety of the token stream should be parsed and accounted for.
  if(!eos()) {
    // Possibly this should be an exception and not a SyntaxError.
    throw_syntax_error("End of file reached without parsing everything.");
  }

  return ast;
}

auto AcrisParser::pratt_parse(AccessorFn<PrattParser> t_fn) -> NodePtr
{
  return t_fn(m_pratt);
}

auto AcrisParser::type_parse(AccessorFn<TypeParser> t_fn) -> NodePtr
{
  return t_fn(m_type);
}
} // namespace parser::acris
