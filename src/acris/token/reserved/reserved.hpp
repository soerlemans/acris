#ifndef ACRIS_ACRIS_TOKEN_RESERVED_RESERVED_HPP
#define ACRIS_ACRIS_TOKEN_RESERVED_RESERVED_HPP

// STL Includes:
#include <map>
#include <type_traits>
#include <utility>

// Library Includes:
#include <boost/assign/list_of.hpp>
#include <boost/bimap.hpp>

// Local Includes:
#include "terminal.hpp"


// Macros:
#define DEFINE_TERMINAL(t_name, t_id, t_token) \
  constexpr Terminal t_name                    \
  {                                            \
    t_id, TokenType::t_token                   \
  }

namespace token::reserved {
// Aliases:
template<typename Key = std::string_view>
using TerminalMap = boost::bimap<Key, TokenType>;

// clang-format off
// Language reserved keywords
namespace keywords {
  // Variables:
  DEFINE_TERMINAL(g_let, "let", LET); // Immutable var init.
  DEFINE_TERMINAL(g_var, "var", VAR); // Mutable var init.
  DEFINE_TERMINAL(g_readonly, "ro", READONLY); // Specify a pointer destination as not writeable.

  // Package:
  DEFINE_TERMINAL(g_module, "module", MODULE);
  DEFINE_TERMINAL(g_import, "import", IMPORT);
  DEFINE_TERMINAL(g_export, "export", EXPORT);

	// Meta:
	// TODO: Consider renaming to just decl.
  DEFINE_TERMINAL(g_declare, "decl", DECLARE);
  DEFINE_TERMINAL(g_macro,   "macro",   MACRO);
  DEFINE_TERMINAL(g_const,   "const",   CONST); // Compile time constants.

  // User Types:
  DEFINE_TERMINAL(g_alias,  "alias",  ALIAS);
  DEFINE_TERMINAL(g_enum,   "enum",   ENUM);
  DEFINE_TERMINAL(g_union,   "union",   UNION);
  DEFINE_TERMINAL(g_struct, "struct", STRUCT);
  DEFINE_TERMINAL(g_self,   "self",   SELF);

  DEFINE_TERMINAL(g_proto, "proto", PROTO);

  // Control statements:
  DEFINE_TERMINAL(g_func,  "func",  FUNCTION);
  DEFINE_TERMINAL(g_meth,  "meth",  METHOD);
  DEFINE_TERMINAL(g_match, "match", MATCH);
  DEFINE_TERMINAL(g_if,    "if",    IF);
  DEFINE_TERMINAL(g_else,  "else",  ELSE);
  DEFINE_TERMINAL(g_elif,  "elif",  ELIF);
  DEFINE_TERMINAL(g_loop,  "loop",  LOOP);

  // Jump:
  DEFINE_TERMINAL(g_break,    "break",    BREAK);
  DEFINE_TERMINAL(g_continue, "continue", CONTINUE);
  DEFINE_TERMINAL(g_defer,    "defer",    DEFER);
  DEFINE_TERMINAL(g_return,   "return",   RETURN);

  // Literals:
  DEFINE_TERMINAL(g_true,  "true",  TRUE);
  DEFINE_TERMINAL(g_false, "false", FALSE);

	// Primitives:
  DEFINE_TERMINAL(g_void,  "void",  VOID);

  DEFINE_TERMINAL(g_f32,  "f32",   F32);
  DEFINE_TERMINAL(g_f64,  "f64",   F64);

  DEFINE_TERMINAL(g_int, "int", INT);
  DEFINE_TERMINAL(g_i8,  "i8",  I8);
  DEFINE_TERMINAL(g_i16, "i16", I16);
  DEFINE_TERMINAL(g_i32, "i32", I32);
  DEFINE_TERMINAL(g_i64, "i64", I64);
  DEFINE_TERMINAL(g_isz, "isz", ISIZE);

  DEFINE_TERMINAL(g_uint, "uint", UINT);
  DEFINE_TERMINAL(g_u8,   "u8",   U8);
  DEFINE_TERMINAL(g_u16,  "u16",  U16);
  DEFINE_TERMINAL(g_u32,  "u32",  U32);
  DEFINE_TERMINAL(g_u64,  "u64",  U64);
  DEFINE_TERMINAL(g_usz,  "usz",  USIZE);

  DEFINE_TERMINAL(g_char, "char",  CHAR);
  DEFINE_TERMINAL(g_cstr, "cstr",  CSTR);

  // FIXME: Make builtin types part of the lexer.
  // Currently we dont do this.
  // Each builtin type should have its own TokenType.

  // const std::map g_builtin_types {
  // };

  // TODO: Convert keywords to boost::bimap.
  // const TerminalMap g_keywords2 = boost::assign::list_of<TerminalMap<std::string_view>::relation>
  // 	(g_let.pair())
  // 	(g_var.pair())
  // 	(g_module.pair()) (g_import.pair()) (g_priv.pair()) (g_pub.pair())
  // 	(g_struct.pair()) (g_proto.pair()) (g_impl.pair())
  //    (g_func.pair())
  // 	(g_match.pair())
  //    (g_if.pair()) (g_else.pair()) (g_elif.pair())
  //    (g_loop.pair())
  //    (g_break.pair()) (g_continue.pair()) (g_defer.pair()) (g_return.pair())
  //    (g_true.pair()) (g_false.pair())
  //    ;

  const std::map g_keywords {
    g_let.pair(), g_var.pair(), g_readonly.pair(),
    g_module.pair(), g_import.pair(), g_export.pair(),
    g_declare.pair(), g_macro.pair(), g_const.pair(),
    g_alias.pair(), g_enum.pair(), g_union.pair(), g_struct.pair(), g_self.pair(),
		g_proto.pair(),
    g_func.pair(),
    g_match.pair(),
    g_if.pair(), g_else.pair(), g_elif.pair(),
    g_loop.pair(),
    g_break.pair(), g_continue.pair(), g_defer.pair(), g_return.pair(),
    g_true.pair(), g_false.pair(),

		g_void.pair(),
		g_f32.pair(), g_f64.pair(),
		g_int.pair(), g_i8.pair(), g_i16.pair(), g_i32.pair(), g_i64.pair(), g_isz.pair(),
		g_uint.pair(), g_u8.pair(), g_u16.pair(), g_u32.pair(), g_u64.pair(), g_usz.pair(),
		g_char.pair(), g_cstr.pair()
};
} // namespace keywords

// Language reserved symbols
namespace symbols {
  // Meta:
  DEFINE_TERMINAL(g_hash, '#', HASH);

  DEFINE_TERMINAL(g_attribute_open,  "[[", ATTRIBUTE_OPEN);
  DEFINE_TERMINAL(g_attribute_close, "]]", ATTRIBUTE_CLOSE);

  // Braces
  DEFINE_TERMINAL(g_paren_open,     '(', PAREN_OPEN);
  DEFINE_TERMINAL(g_paren_close,    ')', PAREN_CLOSE);
  DEFINE_TERMINAL(g_accolade_open,  '{', ACCOLADE_OPEN);
  DEFINE_TERMINAL(g_accolade_close, '}', ACCOLADE_CLOSE);
  DEFINE_TERMINAL(g_bracket_open,     '[', BRACKET_OPEN);
  DEFINE_TERMINAL(g_bracket_close,    ']', BRACKET_CLOSE);

  // Arithmetic operators:
  DEFINE_TERMINAL(g_plus,         '+', PLUS);
  DEFINE_TERMINAL(g_minus,        '-', MINUS);
  DEFINE_TERMINAL(g_asterisk,     '*', ASTERISK);
  DEFINE_TERMINAL(g_slash,        '/', SLASH);
  DEFINE_TERMINAL(g_percent_sign, '%', PERCENT_SIGN);

  DEFINE_TERMINAL(g_increment, "++", INCREMENT);
  DEFINE_TERMINAL(g_decrement, "--", DECREMENT);

  // Assignment:
  DEFINE_TERMINAL(g_mul_assign, "*=", MUL_ASSIGN);
  DEFINE_TERMINAL(g_div_assign, "/=", DIV_ASSIGN);
  DEFINE_TERMINAL(g_mod_assign, "%=", MOD_ASSIGN);

  DEFINE_TERMINAL(g_add_assign, "+=", ADD_ASSIGN);
  DEFINE_TERMINAL(g_sub_assign, "-=", SUB_ASSIGN);

  DEFINE_TERMINAL(g_assignment, '=',  ASSIGNMENT);

  // Comparisons:
  DEFINE_TERMINAL(g_less_than,       '<',  LESS_THAN);
  DEFINE_TERMINAL(g_less_than_equal, "<=", LESS_THAN_EQUAL);

  DEFINE_TERMINAL(g_equal,     "==", EQUAL);
  DEFINE_TERMINAL(g_not_equal, "!=", NOT_EQUAL);

  DEFINE_TERMINAL(g_greater_than,       '>',  GREATER_THAN);
  DEFINE_TERMINAL(g_greater_than_equal, ">=", GREATER_THAN_EQUAL);

  // Logic:
  DEFINE_TERMINAL(g_not, '!',  NOT);
  DEFINE_TERMINAL(g_or,  "||", OR);
  DEFINE_TERMINAL(g_and, "&&", AND);

  // Miscellaneous:
  DEFINE_TERMINAL(g_arrow,         "->", ARROW);
  DEFINE_TERMINAL(g_dot,           '.',  DOT);
  DEFINE_TERMINAL(g_comma,         ',',  COMMA);
  DEFINE_TERMINAL(g_question_mark, '?',  QUESTION_MARK);
  DEFINE_TERMINAL(g_double_colon,  "::", DOUBLE_COLON);
  DEFINE_TERMINAL(g_colon,         ':',  COLON);
  DEFINE_TERMINAL(g_semicolon,     ';',  SEMICOLON);
  DEFINE_TERMINAL(g_ampersand,     '&',  AMPERSAND);

  DEFINE_TERMINAL(g_newline,  '\n', NEWLINE);

  const std::map g_single_symbols {
		g_hash.pair(),

    g_paren_open.pair(),
    g_paren_close.pair(),
    g_accolade_open.pair(),
    g_accolade_close.pair(),
    g_bracket_open.pair(),
    g_bracket_close.pair(),

    g_assignment.pair(),

    g_less_than.pair(),
    g_greater_than.pair(),

    g_not.pair(),

    g_dot.pair(),
    g_comma.pair(),
    g_question_mark.pair(),
    g_colon.pair(),
    g_semicolon.pair(),

    g_plus.pair(),
    g_minus.pair(),
    g_asterisk.pair(),
    g_slash.pair(),
    g_percent_sign.pair(),
		g_ampersand.pair(),

    g_newline.pair()
  };

  const std::map g_multi_symbols{
    g_attribute_open.pair(),
    g_attribute_close.pair(),

    g_increment.pair(),
    g_decrement.pair(),

    g_mul_assign.pair(),
    g_div_assign.pair(),
    g_mod_assign.pair(),

    g_add_assign.pair(),
    g_sub_assign.pair(),

    g_less_than_equal.pair(),

    g_equal.pair(),
    g_not_equal.pair(),

    g_greater_than_equal.pair(),

    g_or.pair(),
    g_and.pair(),

    g_arrow.pair(),
    g_double_colon.pair()
  };

  namespace none {
    constexpr auto g_single_quote{'\''};
    constexpr auto g_double_quote{'"'};
    constexpr auto g_backslash{'\\'};

    // These are for dealing with comments:
    constexpr auto g_exclamation_mark{'!'};
    constexpr auto g_slash{'/'};
    constexpr auto g_asterisk{'*'};
    constexpr auto g_newline{'\n'};
  }
} // namespace symbols
} // namespace reserved

// clang-format on

#endif // ACRIS_ACRIS_TOKEN_RESERVED_RESERVED_HPP
