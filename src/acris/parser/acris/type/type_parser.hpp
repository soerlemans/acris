#ifndef ACRIS_ACRIS_PARSER_ACRIS_TYPE_TYPE_PARSER_HPP
#define ACRIS_ACRIS_PARSER_ACRIS_TYPE_TYPE_PARSER_HPP

// STL Includes:
#include <memory>
#include <unordered_map>
#include <utility>

// Absolute Includes:
#include "acris/parser/parser.hpp"
#include "lib/stdtypes.hpp"

namespace parser::type {
// Classes:
class TypeParser : public Parser {
  public:
  explicit TypeParser(ParserContextPtr t_ctx);

  // virtual auto type_ref() -> NodePtr; // TODO.
  virtual auto type_pointer() -> NodePtr;
  virtual auto type_array() -> NodePtr;
  virtual auto type_id_builtin() -> NodePtr;
  virtual auto type_id_user() -> NodePtr;
  virtual auto type_id() -> NodePtr;

  virtual auto type_expr() -> NodePtr;

  virtual ~TypeParser() = default;
};
} // namespace parser::type

#endif // ACRIS_ACRIS_PARSER_ACRIS_TYPE_TYPE_PARSER_HPP
