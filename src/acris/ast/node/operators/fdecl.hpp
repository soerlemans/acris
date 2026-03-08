#ifndef ACRIS_ACRIS_AST_NODE_OPERATORS_FDECL_HPP
#define ACRIS_ACRIS_AST_NODE_OPERATORS_FDECL_HPP

// Forward Declarations:
namespace ast::node::operators {
// UnaryOperator derived classes:
class AddressOf;
class Dereference;
class UnaryPrefix;
class ToCast;
class Increment;
class Decrement;
class Not;

// BinaryOperator derived classes:
class Arithmetic;
class Comparison;
class And;
class Or;
class Assignment;


// Trinary operator:
class Ternary;
} // namespace ast::node::operators

#endif // ACRIS_ACRIS_AST_NODE_OPERATORS_FDECL_HPP
