#ifndef ACRIS_ACRIS_SEMANTIC_SEMANTIC_CHECKER_HPP
#define ACRIS_ACRIS_SEMANTIC_SEMANTIC_CHECKER_HPP

// STL Includes:
#include <list>
#include <string_view>

// Absolute includes:
#include "acris/ast/node/node_traits/attribute_data.hpp"
#include "acris/ast/node/node_traits/type_data.hpp"
#include "acris/ast/visitor/node_visitor.hpp"
#include "acris/container/text_position.hpp"
#include "acris/types/semantic/semantic.hpp"
#include "acris/types/semantic/symbol.hpp"

// Local Includes:
#include "annotation_queue.hpp"
#include "semantic_validator.hpp"
#include "symbol_env_state.hpp"
#include "type_node_evaluator.hpp"
#include "type_promoter.hpp"

/*!
 * @file
 *
 * Contains class responsible for type checking the ast.
 * This class will also annotate the AST and perform type promotion.
 * As well as generate a global @ref SymbolTable.
 * Which can be used for symbol lookup.
 */

namespace semantic {
// Using:
namespace node = ast::node;

using ast::visitor::Any;
using ast::visitor::NodeVisitor;
using container::TextPosition;
using node::NodeListPtr;
using node::NodePtr;
using node::node_traits::AttributeData;
using node::node_traits::AttributeMetadata;
using node::node_traits::AttributeSeq;
using node::node_traits::TypeData;
using types::core::NativeType;
using types::core::NativeTypeOpt;
using types::symbol::MemberSymbol;
using types::symbol::MethodSymbol;
using types::symbol::SymbolData;
using types::symbol::SymbolDataList;

// Aliases:
//! Stores active contexts.
using AttributeContext = AttributeSeq;
using ActiveStruct = std::optional<types::symbol::StructTypePtr>;

// Classes:
// TODO: Add check for checking if the AST only has a single module
// declaration. We should either way, have some kind of entity which merges
// multiple translation units. Into a single Module Unit. Maybe force module
// declarations to be at the top of the file? To be considered when enforcing
// modules.
/*!
 * The semantic checker validates the semantic validity of the AST.
 * This performs type inference and type checking.
 * This also checks that we are not assigning to any constant variables.
 * We also check if arguments have the right types for any functions.
 *
 * We also create a symbol table after semantic analysis.
 * As we need to create it anyway when looking up symbols.
 */
class SemanticChecker : public NodeVisitor {
  private:
  SemanticValidator m_validator;

  SymbolEnvState m_symbol_state;
  TypeNodeEvaluator m_node_evaluator;
  AttributeContext m_active_attrs;

  // Used to resolve self to type.
  ActiveStruct m_active_struct;

  AnnotationQueue m_annot_queue;

  protected:
  // Environment related methods:
  auto push_env() -> void;
  auto pop_env() -> void;
  auto clear_env() -> void;

  auto str2symbol_data(std::string_view t_type_id) -> SymbolData;
  auto node2symbol_data(NodePtr t_type_node) -> SymbolData;

  /*!
   * Annotate a node with its attribute data.
   */
  auto annotate_attr(AttributeData* t_node) -> void;

  /*!
   * Queue annotation for later.
   */
  auto queue_annotation(TypeData* t_node, const SymbolData& t_data) -> void;

  auto add_symbol_declaration(std::string_view t_id, const SymbolData& t_data)
    -> void;

  /*!
   * Add symbol to current @ref EnvSate.
   * Also add the symbol to the global @ref SymbolTable.
   *
   * @return false means inserting the symbol failed.
   */
  auto add_symbol_definition(std::string_view t_id, const SymbolData& t_data)
    -> void;

  auto add_struct_member_definition(SymbolData& t_struct,
                                    const MemberSymbol& t_sym) -> void;

  auto add_struct_method_definition(SymbolData& t_struct,
                                    const MethodSymbol& t_sym) -> void;

  [[nodiscard("Pure method must use result.")]]
  auto get_symbol_data_from_env(std::string_view t_id) -> SymbolData&;

  // Helper methods for type promotion:
  //! Handle type conversion for conditionals.
  auto handle_condition(const SymbolData& t_data,
                        const TextPosition& t_pos) const -> void;

  //! Handle type promotion between two different types.
  auto promote(const SymbolData& t_lhs, const SymbolData& rhs,
               PromotionMode t_mode = PromotionMode::PROMOTE_TO_LHS) const
    -> NativeTypeOpt;

  // Helper methods for dealing with resolving nodes to SymbolData:
  // NodeVisitor visitation is not marked const so these methods cant be const.
  auto get_symbol_data(NodePtr t_ptr) -> SymbolData;
  auto get_resolved_result_type(NodePtr t_ptr) -> SymbolData;
  auto get_type_list(NodeListPtr t_list) -> SymbolDataList;
  auto get_resolved_result_type_list(NodeListPtr t_list) -> SymbolDataList;

  public:
  SemanticChecker();

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
  auto visit(node::lvalue::IdentifierNode* t_id) -> Any override;
  auto visit(node::lvalue::Subscript* t_subscript) -> Any override;
  auto visit(node::lvalue::ScopeResolution* t_scope_res) -> Any override;

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
  auto visit(node::operators::ToCast* t_cast) -> Any override;

  // Logical:
  auto visit(node::operators::Not* t_not) -> Any override;
  auto visit(node::operators::And* t_and) -> Any override;
  auto visit(node::operators::Or* t_or) -> Any override;

  // auto visit(node::operators::Ternary* t_ternary) -> Any override;

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

  // User Types:
  auto visit(node::user_types::EnumField* t_enum) -> Any override;
  auto visit(node::user_types::Enum* t_field) -> Any override;
  auto visit(node::user_types::Method* t_meth) -> Any override;
  auto visit(node::user_types::Interface* t_ifc) -> Any override;
  auto visit(node::user_types::MemberDecl* t_meth) -> Any override;
  auto visit(node::user_types::Struct* t_struct) -> Any override;
  auto visit(node::user_types::Self* t_self) -> Any override;
  auto visit(node::user_types::Member* t_member) -> Any override;
  auto visit(node::user_types::MemberAccess* t_access) -> Any override;

  auto check(NodePtr t_ast) -> void;

  virtual ~SemanticChecker() = default;
};
} // namespace semantic

#endif // ACRIS_ACRIS_SEMANTIC_SEMANTIC_CHECKER_HPP
