#ifndef ACRIS_ACRIS_PARSER_ACRIS_PRATT_BINDING_MAPS_HPP
#define ACRIS_ACRIS_PARSER_ACRIS_PRATT_BINDING_MAPS_HPP

// Local Includes:
#include "binding_map.hpp"

namespace parser::pratt::binding {
// Parenthesis always have the highest precedence, so there is no need to assign
// them a binding power!

// Classes:
//! This class is a map of prefix operator binding powers.
class PrefixMap : public BindingMap {
  public:
  PrefixMap()
  {
    // Dereference:
    INSERT_BINDING(ASTERISK, 100, 99); // Dereference.
    INSERT_BINDING(AMPERSAND, 100, 99); // Address off.

    // Unary prefix operators:
    INSERT_BINDING(NOT, 90, 90);
    INSERT_BINDING(PLUS, 90, 90);
    INSERT_BINDING(MINUS, 90, 90);
  }
};

//! This class is a map of infix operator binding powers.
class InfixMap : public BindingMap {
  public:
  InfixMap()
  {
    // Member access:
    INSERT_BINDING(DOT, 110, 109);   // Member Access.
    INSERT_BINDING(ARROW, 110, 109); // Member Access via pointer.

    // Factor:
    INSERT_BINDING(ASTERISK, 69, 70);     // Multiplication
    INSERT_BINDING(SLASH, 69, 70);        // Division
    INSERT_BINDING(PERCENT_SIGN, 69, 70); // Modulo

    // Arithmetic:
    INSERT_BINDING(PLUS, 59, 60);  // Addition
    INSERT_BINDING(MINUS, 59, 60); // Subtraction

    // Comparisons:
    INSERT_BINDING(LESS_THAN, 50, 50);
    INSERT_BINDING(LESS_THAN_EQUAL, 50, 50);
    INSERT_BINDING(EQUAL, 50, 50);
    INSERT_BINDING(NOT_EQUAL, 50, 50);
    INSERT_BINDING(GREATER_THAN, 50, 50);
    INSERT_BINDING(GREATER_THAN_EQUAL, 50, 50);

    // Logical
    INSERT_BINDING(AND, 39, 40);
    INSERT_BINDING(OR, 29, 30);

    // Ternary:
    // INSERT_BINDING(QUESTION_MARK, 20, 19);

    // Assignments:
    // TODO: Remove? We dont treat assignment as an expression but instead a
    // statement.
    INSERT_BINDING(MUL_ASSIGN, 10, 9);
    INSERT_BINDING(DIV_ASSIGN, 10, 9);
    INSERT_BINDING(MOD_ASSIGN, 10, 9);
    INSERT_BINDING(ADD_ASSIGN, 10, 9);
    INSERT_BINDING(SUB_ASSIGN, 10, 9);
    INSERT_BINDING(ASSIGNMENT, 10, 9);
  }
};

class PostfixMap : public BindingMap {
  public:
  PostfixMap()
  {
    INSERT_BINDING(BRACKET_OPEN, 120, 119);
    INSERT_BINDING(PAREN_OPEN, 120, 119);

		// Member access is postfix.
    INSERT_BINDING(DOT, 110, 109);
    INSERT_BINDING(ARROW, 110, 109);

    // Casting sits inbetween tightly binded expression
    INSERT_BINDING(TO, 80, 80);
  }
};
} // namespace parser::pratt::binding

#endif // ACRIS_ACRIS_PARSER_ACRIS_PRATT_BINDING_MAPS_HPP
