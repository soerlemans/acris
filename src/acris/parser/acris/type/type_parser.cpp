#include "type_parser.hpp"

// STL Includes:
#include <memory>
#include <stdexcept>

// Absolute Includes:
#include "acris/ast/node/include_nodes.hpp"
#include "acris/debug/log.hpp"

namespace parser::type {
// Using Statements:
NODE_USING_ALL_NAMESPACES()

// Methods:
TypeParser::TypeParser(ParserContextPtr t_ctx): Parser{t_ctx}
{}

auto TypeParser::type_pointer() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto token{get_token()};
  const auto pos{token.position()};
  if(next_if(TokenType::ASTERISK)) {
    PARSER_FOUND(TokenType::ASTERISK);

    // Consume more asterisks to add to indirection.
    usz indirection{1};
    while(next_if(TokenType::ASTERISK)) {
      indirection++;
    }

    // Check if readonly is applied..
    bool readonly{false};
    if(next_if(TokenType::READONLY)) {
      readonly = true;
    }

    auto target_type{type_expr()};

    if(!target_type) {
      throw_syntax_error("Expected a type expression after *.");
    }

    node =
      make_node<Pointer>(pos, std::move(target_type), indirection, readonly);
  }

  return node;
}

auto TypeParser::type_array() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto token{get_token()};
  if(next_if(TokenType::BRACKET_OPEN)) {
    PARSER_FOUND(TokenType::BRACKET_OPEN);

    const auto pos{token.position()};
    auto target_type{type_expr()};

    if(!target_type) {
      throw_syntax_error("Expected a type expression after [.");
    }

    expect(TokenType::SEMICOLON);

    // TODO: In future accept compile time variables for size, figure that out.
    // Possibly it should be a binary operator.
    const auto integer_literal{get_token()};
    expect(TokenType::INTEGER_LITERAL);
    const auto array_size{integer_literal.int_()};

    expect(TokenType::BRACKET_CLOSE);

    node = make_node<Array>(pos, std::move(target_type), (usz)array_size);
  }

  return node;
}

auto TypeParser::type_id_builtin() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto token{get_token()};
  const auto pos{token.position()};

  const auto make_primitive_helper{[&](std::string_view t_id) {
    next(); // Consume primitive/native/builtin type.
    DBG_TRACE_PRINT(INFO, "Primitive type: ", t_id);

    // TODO: Create node for primitive types, this will speed up type lookup.
    // During semantic phase.
    node = make_node<TypeName>(pos, t_id);
  }};

  switch(token.type()) {
    case TokenType::VOID:
      make_primitive_helper("void");
      break;

    case TokenType::F32:
      make_primitive_helper("f32");
      break;

    case TokenType::F64:
      make_primitive_helper("f64");
      break;

    case TokenType::INT:
      make_primitive_helper("int");
      break;

    case TokenType::I8:
      make_primitive_helper("i8");
      break;

    case TokenType::I16:
      make_primitive_helper("i16");
      break;

    case TokenType::I32:
      make_primitive_helper("i32");
      break;

    case TokenType::I64:
      make_primitive_helper("i64");
      break;

    case TokenType::ISIZE:
      make_primitive_helper("isz");
      break;

    case TokenType::UINT:
      make_primitive_helper("uint");
      break;

    case TokenType::U8:
      make_primitive_helper("u8");
      break;

    case TokenType::U16:
      make_primitive_helper("u16");
      break;

    case TokenType::U32:
      make_primitive_helper("u32");
      break;

    case TokenType::U64:
      make_primitive_helper("u64");
      break;

    case TokenType::USIZE:
      make_primitive_helper("usz");
      break;

    case TokenType::CHAR:
      make_primitive_helper("char");
      break;

    case TokenType::CSTR: {
      make_primitive_helper("cstr");
      break;
    }

    default:
      // Do nothing, not a primitive..
      break;
  }

  return node;
}

auto TypeParser::type_id_user() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  const auto token{get_token()};
  if(next_if(TokenType::IDENTIFIER)) {
    const auto pos{token.position()};
    const auto id{token.str()};

    DBG_TRACE_PRINT(INFO, "TypeName: ", id);
    node = make_node<TypeName>(pos, id);
  }

  return node;
}

auto TypeParser::type_id() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(auto ptr{type_id_builtin()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{type_id_user()}; ptr) {
    node = std::move(ptr);
  }

  return node;
}

auto TypeParser::type_expr() -> NodePtr
{
  DBG_TRACE_FN(VERBOSE);
  NodePtr node{};

  if(auto ptr{type_pointer()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{type_array()}; ptr) {
    node = std::move(ptr);
  } else if(auto ptr{type_id()}; ptr) {
    node = std::move(ptr);
  }

  return node;
}
} // namespace parser::type
