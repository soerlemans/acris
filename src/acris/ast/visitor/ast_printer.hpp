#ifndef ACRIS_ACRIS_AST_VISITOR_AST_PRINTER_HPP
#define ACRIS_ACRIS_AST_VISITOR_AST_PRINTER_HPP

// STL Includes:
#include <concepts>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string_view>

// Library Includes:
#include <rang.hpp>

// Absolute Includes:
#include "lib/is_any_of.hpp"

// Relative AST Includes:
#include "../node/include_nodes.hpp"

// Local Includes:
#include "node_visitor.hpp"

namespace ast::visitor {
// Classes:
/*!
 * Visitor made for printing the AST Node per node.
 * Has a unique overload for every print.
 */
class AstPrinter : public NodeVisitor {
  private:
  int m_counter{0};
  std::ostream& m_os;

  protected:
  //! Prints the given @ref t_str only if the @ref t_ptr is true.
  auto print_if(std::string_view t_str, ast::node::NodePtr t_ptr) -> void;

  template<typename... Args>
  auto print(Args&&... t_args) -> void
  {
    // Construct the prefix
    m_os << std::string((std::size_t)m_counter, ' ') << "-> ";

    // Print the arguments
    (m_os << ... << t_args);

    // Create the indentation level denoter
    m_os << " (indent: " << m_counter << ")\n";
  }

  template<typename... Args>
  auto print_hl(Args&&... t_args) -> void
  {
    using namespace rang;

    // Used to highlight nodes.
    print(style::bold, style::underline, std::forward<Args>(t_args)...,
          style::reset);
  }

  template<typename Base, typename Ptr, typename Fn>
  auto when_derived(Ptr t_ptr, const Fn t_fn) -> bool
  {
    using Type = std::remove_pointer<decltype(t_ptr)>::type;

    bool is_derived{false};
    if constexpr(std::derived_from<Type, Base>) {
      t_fn(t_ptr);
      is_derived = true;
    }

    return is_derived;
  }

  // TODO: There is likely a more elegant solution to this, but for now this
  // works.
  template<typename Ptr>
  auto print_traits(Ptr t_ptr) -> void
  {
    using namespace ast::node::node_traits;

    const auto lambda{[this](const auto t_vw, auto t_any) {
      using namespace ast::node;

      using Type = std::remove_pointer<decltype(t_any)>::type;

      std::stringstream ss;
      ss << "| " << t_vw << ": ";

      if constexpr(lib::IsAnyOf<Type, NodePtr, NodeListPtr>) {
        print_if(ss.str(), t_any);
      } else {
        print(ss.str(), t_any);
      }
    }};

    when_derived<Identifier>(t_ptr, [&](auto t_ptr) {
      lambda("Identifier", t_ptr->identifier());
    });

    when_derived<InitExpr>(t_ptr, [&](auto t_ptr) {
      lambda("Init Expr", t_ptr->init_expr());
    });

    when_derived<Condition>(t_ptr, [&](auto t_ptr) {
      lambda("Condition", t_ptr->condition());
    });

    when_derived<Expr>(t_ptr, [&](auto t_ptr) {
      lambda("Expr", t_ptr->expr());
    });

    when_derived<Params>(t_ptr, [&](auto t_ptr) {
      lambda("Params", t_ptr->params());
    });

    when_derived<TypeAnnotation>(t_ptr, [&](auto t_ptr) {
      lambda("Type Annotation: ", t_ptr->type());

      // print("| Type Annotation:");

      // const auto type{t_ptr->type()};
      // if(type) {
      //   traverse(t_ptr->type());
      // } else {
      //   print("Nil");
      // }
    });

    when_derived<TypeData>(t_ptr, [&](auto t_ptr) {
      print("| Type Data: ", t_ptr->get_type());
    });

    when_derived<AttributeData>(t_ptr, [&](auto t_ptr) {
      print("| Attribute Data: ", t_ptr->get_attributes());
    });

    when_derived<Body>(t_ptr, [&](auto t_ptr) {
      lambda("Body", t_ptr->body());
    });

    when_derived<Then>(t_ptr, [&](auto t_ptr) {
      lambda("Then", t_ptr->then());
    });

    when_derived<Alt>(t_ptr, [&](auto t_ptr) {
      lambda("Alt", t_ptr->alt());
    });

    when_derived<UnaryOperator>(t_ptr, [&](auto t_ptr) {
      lambda("Left", t_ptr->left());
    });

    when_derived<BinaryOperator>(t_ptr, [&](auto t_ptr) {
      lambda("Right", t_ptr->right());
    });
  }

  public:
  AstPrinter(std::ostream& t_os);

  // Control:
  auto visit(node::control::If* t_if) -> Any override;
  auto visit(node::control::Loop* t_loop) -> Any override;
  auto visit(node::control::Continue* t_continue) -> Any override;
  auto visit(node::control::Break* t_break) -> Any override;
  auto visit(node::control::Defer* t_defer) -> Any override;
  auto visit(node::control::Return* t_return) -> Any override;

  // Function:
  auto visit(node::function::Parameter* t_param) -> Any override;
  auto visit(node::function::Function* t_fn) -> Any override;
  auto visit(node::function::FunctionCall* t_fn_call) -> Any override;
  auto visit(node::function::ReturnType* t_rt) -> Any override;

  // Lvalue:
  auto visit(node::lvalue::Let* t_let) -> Any override;
  auto visit(node::lvalue::Var* t_var) -> Any override;
  auto visit(node::lvalue::Variable* t_var) -> Any override;
  auto visit(node::lvalue::Subscript* t_subscript) -> Any override;

  // Meta:
  auto visit(node::meta::Attribute* t_attr) -> Any override;
  auto visit(node::meta::LetDecl* t_ldecl) -> Any override;
  auto visit(node::meta::VarDecl* t_vdecl) -> Any override;
  auto visit(node::meta::FunctionDecl* t_fdecl) -> Any override;

  // Operators:
  auto visit(node::operators::Arithmetic* t_arith) -> Any override;
  auto visit(node::operators::Assignment* t_assign) -> Any override;
  auto visit(node::operators::Comparison* t_comp) -> Any override;

  auto visit(node::operators::Increment* t_inc) -> Any override;
  auto visit(node::operators::Decrement* t_dec) -> Any override;

  auto visit(node::operators::AddressOf* t_addr_of) -> Any override;
  auto visit(node::operators::Dereference* t_deref) -> Any override;
  auto visit(node::operators::UnaryPrefix* t_up) -> Any override;

  // Logical:
  auto visit(node::operators::Not* t_not) -> Any override;
  auto visit(node::operators::And* t_and) -> Any override;
  auto visit(node::operators::Or* t_or) -> Any override;

  auto visit(node::operators::Ternary* t_ternary) -> Any override;

  // Packaging:
  auto visit(node::packaging::Import* t_import) -> Any override;
  auto visit(node::packaging::ModuleDecl* t_mod) -> Any override;

  // Rvalue:
  auto visit(node::rvalue::Float* t_float) -> Any override;
  auto visit(node::rvalue::Integer* t_int) -> Any override;
  auto visit(node::rvalue::Char* t_ch) -> Any override;
  auto visit(node::rvalue::String* t_str) -> Any override;
  auto visit(node::rvalue::ArrayExpr* t_arr) -> Any override;
  auto visit(node::rvalue::Boolean* t_bool) -> Any override;

  // Builtin Types:
  auto visit(node::builtin_types::Pointer* t_ptr) -> Any override;
  auto visit(node::builtin_types::Array* t_arr) -> Any override;
  auto visit(node::builtin_types::TypeName* t_type) -> Any override;

  // User Types:
  auto visit(node::user_types::EnumField* t_enum) -> Any override;
  auto visit(node::user_types::Enum* t_field) -> Any override;
  auto visit(node::user_types::Method* t_meth) -> Any override;
  auto visit(node::user_types::MethodCall* t_meth_call) -> Any override;
  auto visit(node::user_types::Interface* t_ifc) -> Any override;
  auto visit(node::user_types::MemberDecl* t_meth) -> Any override;
  auto visit(node::user_types::Struct* t_struct) -> Any override;
  auto visit(node::user_types::Self* t_self) -> Any override;
  auto visit(node::user_types::Member* t_member) -> Any override;
  auto visit(node::user_types::MemberAccess* t_access) -> Any override;

  // Misc:
  auto visit(node::List* t_list) -> Any override;
  auto visit(node::Nil* t_nil) -> Any override;

  auto print(NodePtr t_ast) -> void;

  virtual ~AstPrinter() = default;
};
} // namespace ast::visitor

#endif // ACRIS_ACRIS_AST_VISITOR_AST_PRINTER_HPP
