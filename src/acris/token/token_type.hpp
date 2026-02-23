#ifndef ACRIS_ACRIS_TOKEN_TOKEN_TYPE_HPP
#define ACRIS_ACRIS_TOKEN_TOKEN_TYPE_HPP

// STL Includes:
#include <optional>

namespace token {
// forward declarations:
enum class TokenType;

// Aliases:
using TokenTypeOpt = std::optional<TokenType>;

// Enums:
enum class TokenType {
  // Variables:
  IDENTIFIER = 0,
  LET,
  VAR,
  SCOPE_RESOLUTION,

  // Variable interaction specifiers.
  READONLY,
  WRITEONLY,
  READWRITE,

  // Package:
  MODULE,
  IMPORT,
  EXPORT,

  // Meta:
  HASH, // Used for preprocessor directives.
  ATTRIBUTE_OPEN,
  ATTRIBUTE_CLOSE,

  DECLARE,
  MACRO,
  CONST,
  COMP,

  // User Types:
  ALIAS,
  ENUM,
  IOTA,
  UNION,
  STRUCT,
  SELF,

  PROTO,

  // Control statements:
  SLOT,
  FUNCTION,
  METHOD,
  MATCH,
  SWITCH,
  IF,
  ELSE,
  ELIF,
  LOOP,

  // Jump:
  BREAK,
  CONTINUE,
  DEFER,
  RETURN,

  // Literals:
  INTEGER_LITERAL,
  FLOAT_LITERAL,
  CHAR_LITERAL,
  STRING_LITERAL, // CString (Const String literal).
  BOOL_LITERAL,

  // Logical:
  TRUE,
  FALSE,
  NULL_LIT,

  // TODO: Define C types for interop.

  // Primitves:
  VOID,

  //- Floats:
  F32,
  F64,

  //- Integers:
  INT,
  I8,
  I16,
  I32,
  I64,
  ISIZE,

  UINT,
  U8,
  U16,
  U32,
  U64,
  USIZE,

  //- String:
  CHAR,
  CSTR,

  // Braces:
  PAREN_OPEN,
  PAREN_CLOSE,

  ACCOLADE_OPEN,
  ACCOLADE_CLOSE,

  BRACKET_OPEN,
  BRACKET_CLOSE,

  // Arithmetic:
  PLUS,
  MINUS,
  ASTERISK,
  SLASH,
  PERCENT_SIGN,

  INCREMENT,
  DECREMENT,

  // Assignment:
  MUL_ASSIGN,
  DIV_ASSIGN,
  MOD_ASSIGN,

  ADD_ASSIGN,
  SUB_ASSIGN,

  BITWISE_NOT_ASSIGN,
  BITWISE_AND_ASSIGN,
  BITWISE_OR_ASSIGN,
  BITWISE_XOR_ASSIGN,
  BITWISE_LHS_ASSIGN,
  BITWISE_RHS_ASSIGN,

  ASSIGNMENT,

  // Comparisons:
  LESS_THAN,
  LESS_THAN_EQUAL,

  EQUAL,
  NOT_EQUAL,

  GREATER_THAN,
  GREATER_THAN_EQUAL,

  // Logic:
  NOT,
  AND,
  OR,

  // Bitwise:
  BITWISE_NOT,
  BITWISE_AND,
  BITWISE_OR,
  BITWISE_XOR,
  BITWISE_LHS,
  BITWISE_RHS,

  // Comments:
  LINE_COMMENT,  // Starts with "//".
  BLOCK_COMMENT, // Starts with "/*" ends with "*/", may span multiple lines.

  // Miscellaneous:
  ARROW,
  DOT,
  COMMA,
  QUESTION_MARK,
  COLON,
  SEMICOLON,
  AMPERSAND,

  NEWLINE
};
} // namespace token

#endif // ACRIS_ACRIS_TOKEN_TOKEN_TYPE_HPP
