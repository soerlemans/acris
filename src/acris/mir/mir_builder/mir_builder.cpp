#include "mir_builder.hpp"

// Absolute Includes:
#include "acris/ast/node/include_nodes.hpp"
#include "acris/debug/log.hpp"
#include "lib/stdexcept/stdexcept.hpp"

// Macros:
#define STUB(t_type)                                              \
  auto MirBuilder::visit([[maybe_unused]] t_type* t_ptr) -> Any   \
  {                                                               \
    DBG_CRITICAL("Method for type ", #t_type, " not overriden!"); \
                                                                  \
    return {};                                                    \
  }

namespace mir::mir_builder {
// Using statements:
NODE_USING_ALL_NAMESPACES()

// Methods:
auto MirBuilder::traverse_in_new_env(NodePtr t_node) -> LocalVarEnvState
{
  m_factory->push_env();
  traverse(t_node);
  const auto new_env{m_factory->get_var_env()};
  m_factory->pop_env();

  return new_env;
}

// Public:
MirBuilder::MirBuilder(): m_factory{nullptr}
{}

auto MirBuilder::visit(If* t_if) -> Any
{
  const auto init_expr{t_if->init_expr()};
  const auto cond{t_if->condition()};
  const auto then{t_if->then()};
  const auto alt{t_if->alt()};

  if(init_expr) {
    traverse(init_expr);
  }

  // Conditional jump:
  traverse(cond);
  const auto last_var{m_factory->require_last_var()};

  auto& cond_instr{m_factory->add_instruction(Opcode::COND_JUMP)};
  m_factory->add_result_var({NativeType::BOOL});
  cond_instr.add_operand({last_var});
  const auto cond_result{cond_instr.m_result};

  const auto main_env{m_factory->get_var_env()};

  // Then block:
  auto& then_block{m_factory->add_block("if_then")};
  cond_instr.add_operand({&then_block});

  // TODO: Save environment before, traversal for phi node insertion.
  const auto then_env{traverse_in_new_env(then)};
  const auto then_jump{m_factory->create_instruction(Opcode::JUMP)};

  // Restore to main env.
  m_factory->set_var_env(main_env);

  // TODO: Potentially cleanup?
  if(alt) {
    // Alt block:
    auto& alt_block{m_factory->add_block("if_alt")};
    cond_instr.add_operand({&alt_block});

    const auto alt_env{traverse_in_new_env(alt)};
    const auto alt_jump{m_factory->create_instruction(Opcode::JUMP)};

    // Final block after the if statement.
    auto& merge_block{m_factory->add_block("if_merge")};
    m_factory->insert_jump(then_jump, then_block, merge_block);
    m_factory->insert_jump(alt_jump, alt_block, merge_block);

    // Insert phi nodes.
    const auto merged_env{m_factory->merge_envs(then_env, alt_env)};
    m_factory->set_var_env(merged_env);
  } else {
    // Final block after the if statement.
    auto& merge_block{m_factory->add_block("if_merge")};
    m_factory->insert_jump(then_jump, then_block, merge_block);

    // Insert phi nodes.
    const auto merged_env{m_factory->merge_envs(main_env, then_env)};
    m_factory->set_var_env(main_env);
  }

  return {};
}

auto MirBuilder::visit(Loop* t_loop) -> Any
{
  auto init_expr{t_loop->init_expr()};
  auto cond{t_loop->condition()};
  auto expr{t_loop->expr()};
  auto body{t_loop->body()};

  // TODO: Handle different cases of loop.
  // Currently we expect everything to be there.

  // Make
  traverse(init_expr);
  auto& cond_jump{m_factory->add_instruction(Opcode::JUMP)};
  // TODO: Jump to conditional block.

	// TODO :Treat every block as its own scope where in the header.
	// Everything needs to be merged using phi statements.

  // TODO: Put in own basic block for looping.
  auto& cond_block{m_factory->add_block("loop_cond")};
  cond_jump.add_operand({&cond_block});

  traverse(cond);
  const auto last_var{m_factory->require_last_var()};

  auto& cond_instr{m_factory->add_instruction(Opcode::COND_JUMP)};
  m_factory->add_result_var({NativeType::BOOL});
  cond_instr.add_operand({last_var});
  const auto cond_result{cond_instr.m_result};

  // TODO: Check condition and quit, if condition fails, else to go to merge
  // blcok.

  // TODO: Put in own basic block for looping.
  auto& expr_block{m_factory->add_block("loop_expr")};
  traverse(expr);

  const auto expr_jump{m_factory->create_instruction(Opcode::JUMP)};
  m_factory->insert_jump(expr_jump, expr_block, cond_block);
  // TODO: Jump to start of body block.

  // TODO: Put in own basic block for looping.
  auto& body_block{m_factory->add_block("loop_body")};
  traverse_in_new_env(body);

  const auto end_jump{m_factory->create_instruction(Opcode::JUMP)};
  m_factory->insert_jump(end_jump, body_block, expr_block);

  auto& merge_block{m_factory->add_block("loop_merge")};

  cond_instr.add_operand({&body_block});
  cond_instr.add_operand({&merge_block});

  // Insert phi nodes.
  // const auto merged_env{
  //   m_factory->merge_envs(cond_result, then_env, alt_env)};
  // m_factory->set_var_env(merged_env);

  // Add phi instructions.

  return {};
}

auto MirBuilder::visit([[maybe_unused]] Continue* t_continue) -> Any
{
  // TODO: Replace continue with JUMP?
  m_factory->add_instruction(Opcode::CONTINUE);

  return {};
}

auto MirBuilder::visit([[maybe_unused]] Break* t_break) -> Any
{
  // TODO: Replace break with JUMP?
  m_factory->add_instruction(Opcode::BREAK);

  return {};
}

auto MirBuilder::visit([[maybe_unused]] Defer* t_defer) -> Any
{
  // TODO: Figure this shit out.

  // Defer statements are inserted at the end.
  // Before all return statements.
  // Or at the end of a function.

  // To implement this we need to queue, defer operations.
  // And when they are returned or at the end of the function.
  // Insert all the operations, that we deferred.
  // We should likely use an std::vector for this.

  return {};
}

auto MirBuilder::visit(Return* t_ret) -> Any
{
  // An expression is optional for a return statement.
  auto expr{t_ret->expr()};
  if(expr) {
    traverse(expr);
    const auto last_var{m_factory->require_last_var()};

    auto& ret{m_factory->add_instruction(Opcode::RETURN)};
    ret.add_operand({last_var});
  } else {
    // Add the return with no operand.
    m_factory->add_instruction(Opcode::RETURN);
  }

  return {};
}

// Functions:
auto MirBuilder::visit(Parameter* t_param) -> Any
{
  const auto name{t_param->identifier()};
  const auto type{t_param->get_type()};

  auto param_var{m_factory->create_var(type)};
  auto& current_fn{m_factory->last_function()};

  // Add the just created ssa var to the param list.
  current_fn->m_params.push_back(param_var);

  // Insert an update statement for debugging.
  m_factory->var_bind(name, param_var);

  return {};
}

auto MirBuilder::visit(node::function::Function* t_fn) -> Any
{
  FunctionPtr fn{std::make_shared<Function>()};

  const auto id{t_fn->identifier()};
  const auto params{t_fn->params()};

  const auto fn_type{t_fn->get_type().as_function()};
  const auto body{t_fn->body()};

  // Add the function to the current module.
  fn->m_name = id;

  // Set return type from FnTypePtr.
  fn->m_return_type = fn_type->m_return_type;

  m_factory->add_function_definition(fn);
  m_factory->add_block("entry");

  m_factory->push_env();
  // Visit all the parameters, this adds them to the parameter list.
  traverse(params);

  // Traverse the body.
  traverse(body);
  m_factory->pop_env();

  return {};
}

auto MirBuilder::visit(FunctionCall* t_call) -> Any
{
  const auto fn_name{t_call->identifier()};
  const auto fn_args{t_call->args()};

  const auto args{get_call_args(fn_args)};

  m_factory->add_call(fn_name, args);

  return {};
}

auto MirBuilder::visit([[maybe_unused]] ReturnType* t_rt) -> Any
{
  // TODO: Do something with this?

  return {};
}

// Lvalue:
auto MirBuilder::visit(Let* t_let) -> Any
{
  // The let and var keywords function the same in SSA.
  const auto name{t_let->identifier()};
  const auto type{t_let->get_type()};

  const auto init_expr{t_let->init_expr()};

  const auto source_line{t_let->position().m_line};

  traverse(init_expr);
  auto& bind_instr{m_factory->last_instruction()};
  const auto last_var{m_factory->require_last_var()};

  m_factory->add_comment(source_line);

  m_factory->var_bind(name, last_var);

  return {};
}

auto MirBuilder::visit(Var* t_var) -> Any
{
  // TODO: Cleanup messy.

  // The let and var keywords function the same in SSA.
  const auto name{t_var->identifier()};
  const auto type{t_var->get_type()};

  const auto init_expr{t_var->init_expr()};

  const auto source_line{t_var->position().m_line};

  traverse(init_expr);
  auto& bind_instr{m_factory->last_instruction()};
  const auto last_var{m_factory->require_last_var()};

  m_factory->add_comment(source_line);

  m_factory->var_bind(name, last_var);

  return {};
}

auto MirBuilder::visit(IdentifierNode* t_id) -> Any
{
  const auto name{t_id->identifier()};
  const auto source_line{t_id->position().m_line};

  // TODO: add casting of a variable if it differs from.
  // The source type.

  m_factory->add_variable_ref(name);
  m_factory->add_comment(source_line);

  return {};
}

// Meta:
auto MirBuilder::visit(Attribute* t_attr) -> Any
{
  const auto body{t_attr->body()};

  // TODO: Think about what else to do with attributes, during MIR generation.

  // No need to push and pop env for attribute body.
  // As an attribute body does not nest its scope.
  traverse(body);

  return {};
}

auto MirBuilder::visit(LetDecl* t_ldecl) -> Any
{
  const auto name{t_ldecl->identifier()};
  const auto type{t_ldecl->get_type()};

  m_factory->add_global_declaration(name, type);

  return {};
}

auto MirBuilder::visit(VarDecl* t_vdecl) -> Any
{
  const auto name{t_vdecl->identifier()};
  const auto type{t_vdecl->get_type()};

  m_factory->add_global_declaration(name, type);

  return {};
}

auto MirBuilder::visit(FunctionDecl* t_fdecl) -> Any
{
  FunctionPtr fn{std::make_shared<Function>()};

  const auto id{t_fdecl->identifier()};

  // We only need to add the name for the declaration.
  // During semantic analysis we confirm, that the declaration.
  // Matches the definition.
  fn->m_name = id;

  m_factory->add_function_declaration(fn);

  return {};
}

// Operators:
auto MirBuilder::visit(Arithmetic* t_arith) -> Any
{
  using types::core::is_float;
  using types::core::is_integer;

  // FIXME: Also support type promotion/casting.
  const auto left{t_arith->left()};
  const auto right{t_arith->right()};
  const auto type{t_arith->get_type()};
  const auto source_line{t_arith->position().m_line};

  traverse(left);
  const auto left_var{m_factory->require_last_var()};

  traverse(right);
  const auto right_var{m_factory->require_last_var()};

  const auto add_instr{[&](const Opcode t_opcode) {
    auto& instr{m_factory->add_instruction(t_opcode)};
    m_factory->add_comment(source_line);

    instr.add_operand({left_var});
    instr.add_operand({right_var});

    m_factory->add_result_var(type);
  }};

  const auto add_arithmetic_instr{[&](const Opcode t_iop, const Opcode t_fiop) {
    // TODO: Code for selecting either integer or float instruction.
    // auto opt{type->native_type()};
    // if(opt) {
    //   const auto native_type{opt.value()};
    //   is_float(native_type);

    // } else {
    //   using lib::stdexcept::throw_invalid_argument;

    //   // We only support arithmetic on native types and numerics.
    //   throw_invalid_argument();
    // }
    // if(is_integer()) {
    //   add_instr(t_iop);
    // } else {
    //   add_instr(t_fop);
    // }

    add_instr(t_iop);
  }};

  const auto op{t_arith->op()};
  switch(op) {
    case ArithmeticOp::POWER:
      add_arithmetic_instr(Opcode::IADD, Opcode::FADD);
      break;

    case ArithmeticOp::MULTIPLY:
      add_arithmetic_instr(Opcode::IMUL, Opcode::FMUL);
      break;

    case ArithmeticOp::DIVIDE:
      add_arithmetic_instr(Opcode::IDIV, Opcode::FDIV);
      break;

    case ArithmeticOp::MODULO:
      // FIXME: Check for float argument as its not allowed.
      add_instr(Opcode::IMOD);
      break;

    case ArithmeticOp::ADD:
      add_arithmetic_instr(Opcode::IADD, Opcode::FADD);
      break;

    case ArithmeticOp::SUBTRACT:
      add_arithmetic_instr(Opcode::ISUB, Opcode::FSUB);
      break;

    default:
      using lib::stdexcept::throw_invalid_argument;

      throw_invalid_argument("Arithmetic operator not supported.");
      break;
  }

  return {};
}

auto MirBuilder::visit(Assignment* t_assign) -> Any
{
  using types::core::is_float;
  using types::core::is_integer;

  const auto left{t_assign->left()};
  const auto right{t_assign->right()};
  const auto type{t_assign->get_type()};
  const auto source_line{t_assign->position().m_line};

  // For now we need a dynamic_cast and assume a variable.
  auto* lhs{dynamic_cast<IdentifierNode*>(left.get())};
  if(!lhs) {
    using lib::stdexcept::throw_unexpected_nullptr;

    throw_unexpected_nullptr("Failed to dynamic_cast to Variable*.");
  }

  // TODO: Distinguish between globals and ssa var's.
  const auto name{lhs->identifier()};
  const auto result_var{m_factory->create_var(type)};

  // TODO: Unify with arithmetic, assignment and comparison implementation.
  const auto add_instr{[&](const Opcode t_opcode) {
    // We traverse right side first.
    // This is significant for IR generation of assignment operator.
    traverse(right);
    const auto right_var{m_factory->require_last_var()};

    traverse(left);
    const auto left_var{m_factory->require_last_var()};

    auto& instr{m_factory->add_instruction(t_opcode)};
    instr.add_operand({left_var});
    instr.add_operand({right_var});

    // An assignment assigns to a new var.
    instr.m_result = result_var;
  }};

  const auto add_assignment_instr{[&](const Opcode t_iop, const Opcode t_fiop) {
    // TODO: Left and right are already check to be of the same type.
    // In the semantic checker.
    // So we just need to evaluate if this is an integer or float comparison.
    add_instr(t_iop);

    // Add the final update, for the variable name and comment.
    m_factory->add_update(name, result_var);
    m_factory->add_comment(source_line);
  }};

  // We need to update the left hand side variable.
  const auto op{t_assign->op()};
  switch(op) {
    case AssignmentOp::MULTIPLY:
      add_assignment_instr(Opcode::IMUL, Opcode::FMUL);
      break;

    case AssignmentOp::DIVIDE:
      add_assignment_instr(Opcode::IDIV, Opcode::FDIV);
      break;

    case AssignmentOp::MODULO:
      add_instr(Opcode::IMOD);
      break;

    case AssignmentOp::ADD:
      add_assignment_instr(Opcode::IADD, Opcode::FADD);
      break;

    case AssignmentOp::SUBTRACT:
      add_assignment_instr(Opcode::ISUB, Opcode::FSUB);
      break;

    case AssignmentOp::REGULAR: {
      traverse(right);
      const auto right_var{m_factory->require_last_var()};

      // For a regular assignment just update the ssa env state.
      m_factory->add_update(name, right_var);
      m_factory->add_comment(source_line);
      break;
    }

    default:
      using lib::stdexcept::throw_invalid_argument;

      throw_invalid_argument("Assignment operator not supported.");
      break;
  }

  return {};
}

auto MirBuilder::visit(Comparison* t_comp) -> Any
{
  using types::core::is_float;
  using types::core::is_integer;

  const auto left{t_comp->left()};
  const auto right{t_comp->right()};
  const auto source_line{t_comp->position().m_line};

  traverse(left);
  const auto left_var{m_factory->require_last_var()};

  traverse(right);
  const auto right_var{m_factory->require_last_var()};

  // TODO: Unify with arithmetic, assignment and comparison implementation.
  const auto add_instr{[&](const Opcode t_opcode) {
    auto& instr{m_factory->add_instruction(t_opcode)};
    m_factory->add_comment(source_line);

    instr.add_operand({left_var});
    instr.add_operand({right_var});

    // A comparison always returns the boolean type.
    m_factory->add_result_var({NativeType::BOOL});
  }};

  const auto add_comparison_instr{[&](const Opcode t_iop, const Opcode t_fiop) {
    // TODO: Left and right are already check to be of the same type.
    // In the semantic checker.
    // So we just need to evaluate if this is an integer or float comparison.
    add_instr(t_iop);
  }};

  const auto op{t_comp->op()};
  switch(op) {
    case ComparisonOp::LESS_THAN:
      add_comparison_instr(Opcode::ICMP_LT, Opcode::FCMP_LTE);
      break;

    case ComparisonOp::LESS_THAN_EQUAL:
      add_comparison_instr(Opcode::ICMP_LTE, Opcode::FCMP_LT);
      break;

    case ComparisonOp::EQUAL:
      add_comparison_instr(Opcode::ICMP_EQ, Opcode::FCMP_EQ);
      break;

    case ComparisonOp::NOT_EQUAL:
      add_comparison_instr(Opcode::ICMP_NE, Opcode::FCMP_NE);
      break;

    case ComparisonOp::GREATER_THAN:
      add_comparison_instr(Opcode::ICMP_GT, Opcode::FCMP_GT);
      break;

    case ComparisonOp::GREATER_THAN_EQUAL:
      add_comparison_instr(Opcode::ICMP_GTE, Opcode::FCMP_GTE);
      break;

    default:
      using lib::stdexcept::throw_invalid_argument;

      throw_invalid_argument("Comparison operator not supported.");
      break;
  }

  return {};
}

auto MirBuilder::visit(Increment* t_inc) -> Any
{
  // FIXME: Currently the addition does not update the bindings in
  // m_ssa_var_env.
  const auto left{t_inc->left()};

  traverse(left);
  const auto last_var{m_factory->require_last_var()};

  // TODO: Deduce type, of left.
  auto& inc_instr{m_factory->add_instruction(Opcode::IADD)};
  inc_instr.add_operand({last_var});

  // TODO: Deduce native type.
  Literal lit{NativeType::INT, 1};
  inc_instr.add_operand({lit});

  return {};
}

auto MirBuilder::visit(Decrement* t_dec) -> Any
{
  // FIXME: Currently the subtraction does not update the bindings in
  // m_ssa_var_env.
  const auto left{t_dec->left()};

  traverse(left);
  const auto last_var{m_factory->require_last_var()};

  // TODO: Deduce type, of left.
  auto& dec_instr{m_factory->add_instruction(Opcode::ISUB)};

  dec_instr.add_operand({last_var});

  // TODO: Deduce type, of left.
  Literal lit{NativeType::INT, 1};
  dec_instr.add_operand({lit});

  return {};
}

auto MirBuilder::visit(UnaryPrefix* t_up) -> Any
{
  using ast::node::operators::UnaryPrefixOp;

  const auto op{t_up->op()};
  const auto left{t_up->left()};

  traverse(left);
  const auto last_var{m_factory->require_last_var()};
  const auto type{last_var->m_type};

  switch(op) {
    case UnaryPrefixOp::PLUS: {
      // TODO: Determine if float or integer.
      // TODO: No op.
      break;
    }

    case UnaryPrefixOp::MINUS: {
      // TODO: Determine if float or integer, dont allow unsigned integers.
      auto& sub_instr{m_factory->add_instruction(Opcode::ISUB)};

      Literal lit{NativeType::INT, 0};
      sub_instr.add_operand(lit);
      sub_instr.add_operand(last_var);

      m_factory->add_result_var(type);

      // TODO: Traverse left.
      break;
    }

    default:
      // TODO: THROW!
      break;
  }

  return {};
}

// Logical:
auto MirBuilder::visit(Not* t_not) -> Any
{
  const auto left{t_not->left()};
  const auto source_line{t_not->position().m_line};

  // TODO: Implement.

  return {};
}

auto MirBuilder::visit(And* t_and) -> Any
{
  // TODO: Refactor very messy.

  const auto left{t_and->left()};
  const auto right{t_and->right()};
  const auto source_line{t_and->position().m_line};

  // Jump to enter and flow.
  auto& main_jump{m_factory->add_instruction(Opcode::JUMP)};

  // Left term block:
  auto& left_block{m_factory->add_block("and_left_term")};

  // Create jump for left term block.
  main_jump.add_operand(Label{&left_block});

  traverse(left);
  const auto left_var{m_factory->require_last_var()};

  // Add a conditional jump skipping if left term is false.
  auto& cjmp_instr{m_factory->add_instruction(Opcode::COND_JUMP)};
  auto cjmp_var{m_factory->create_var(NativeType::BOOL)};
  cjmp_instr.m_result = cjmp_var;
  m_factory->add_comment(source_line);

  const auto left_jump{m_factory->create_instruction(Opcode::JUMP)};

  // Right term block:
  auto& right_block{m_factory->add_block("and_right_term")};

  traverse(right);
  const auto right_var{m_factory->require_last_var()};
  const auto right_jump{m_factory->create_instruction(Opcode::JUMP)};

  // Right block:
  auto& merge_block{m_factory->add_block("and_merge")};

  // Finish conditional jump args.
  // We go to the merge_block on false to short circuit.
  cjmp_instr.add_operand(left_var);
  cjmp_instr.add_operand(Label{&right_block});
  cjmp_instr.add_operand(Label{&merge_block});

  m_factory->add_literal(NativeType::BOOL, {false});
  auto false_var{m_factory->require_last_var()};

  auto& phi_instr{m_factory->add_instruction(Opcode::PHI)};
  m_factory->add_result_var({NativeType::BOOL});

  // If we short circuit and go directly to the merge block.
  // Then the result is false.
  phi_instr.add_operand({cjmp_var});
  phi_instr.add_operand({right_var});
  phi_instr.add_operand({false_var}); // Short-circuit false.

  // Insert jumps to merge into the block.
  m_factory->insert_jump(left_jump, left_block, merge_block);
  m_factory->insert_jump(right_jump, right_block, merge_block);

  return {};
}

auto MirBuilder::visit(Or* t_or) -> Any
{
  // TODO: Refactor very messy.

  const auto left{t_or->left()};
  const auto right{t_or->right()};
  const auto source_line{t_or->position().m_line};

  // Jump to enter and flow.
  auto& main_jump{m_factory->add_instruction(Opcode::JUMP)};

  // Left term block:
  auto& left_block{m_factory->add_block("or_left_term")};

  // Create jump for left term block.
  main_jump.add_operand(Label{&left_block});

  traverse(left);
  const auto left_var{m_factory->require_last_var()};

  // Add a conditional jump skipping if left term is false.
  auto& cjmp_instr{m_factory->add_instruction(Opcode::COND_JUMP)};
  auto cjmp_var{m_factory->create_var(NativeType::BOOL)};
  cjmp_instr.m_result = cjmp_var;
  m_factory->add_comment(source_line);

  const auto left_jump{m_factory->create_instruction(Opcode::JUMP)};

  // Right term block:
  auto& right_block{m_factory->add_block("or_right_term")};

  traverse(right);
  const auto right_var{m_factory->require_last_var()};
  const auto right_jump{m_factory->create_instruction(Opcode::JUMP)};

  // Right block:
  auto& merge_block{m_factory->add_block("or_merge")};

  // Finish conditional jump args.
  // We go to the merge_block on true to short circuit.
  cjmp_instr.add_operand(left_var);
  cjmp_instr.add_operand(Label{&merge_block});
  cjmp_instr.add_operand(Label{&right_block});

  m_factory->add_literal(NativeType::BOOL, {true});
  auto true_var{m_factory->require_last_var()};

  auto& phi_instr{m_factory->add_instruction(Opcode::PHI)};
  m_factory->add_result_var({NativeType::BOOL});

  // If we short circuit and go directly to the merge block.
  // Then the result is false.
  phi_instr.add_operand({cjmp_var});
  phi_instr.add_operand({true_var}); // Short-circuit true.
  phi_instr.add_operand({right_var});

  // Insert jumps to merge into the block.
  m_factory->insert_jump(left_jump, left_block, merge_block);
  m_factory->insert_jump(right_jump, right_block, merge_block);

  return {};
}

auto MirBuilder::visit([[maybe_unused]] Ternary* t_ternary) -> Any
{
  return {};
}

// Packaging:
STUB(Import)

auto MirBuilder::visit(ModuleDecl* t_module) -> Any
{
  // For now we just assume there is only one.
  const auto module_name{t_module->identifier()};

  // TODO: Add check to see if the module name has already been set.
  m_factory->set_module_name(module_name);

  return {};
}

// RValue:
auto MirBuilder::visit([[maybe_unused]] Float* t_float) -> Any
{
  // const auto value_f64{t_float->get()}; // Cereal does not support
  const auto value{t_float->get()};

  // Add the literal, which assigns an SSA var for it.
  m_factory->add_literal(NativeType::F32, {value});

  return {};
}

auto MirBuilder::visit(Integer* t_int) -> Any
{
  const auto value_i64{t_int->get()};

  // TODO: Check if it fits ahead of time.
  // FIXME: Cast to int for now (assumed integer literal default type).
  const auto value{(int)value_i64};

  // Add the literal, which assigns an SSA var for it.
  m_factory->add_literal(NativeType::INT, {value});

  return {};
}

auto MirBuilder::visit(String* t_str) -> Any
{
  const std::string value{t_str->get()};

  // Add the literal, which assigns an SSA var for it.
  m_factory->add_literal(NativeType::CSTR, {value});

  return {};
}

auto MirBuilder::visit(Boolean* t_bool) -> Any
{
  const bool value{t_bool->get()};

  // Add the literal, which assigns an SSA var for it.
  m_factory->add_literal(NativeType::BOOL, {value});

  return {};
}

// User Types:
STUB(Method)
STUB(Interface)
STUB(MemberDecl)
STUB(Struct)
STUB(Self)
STUB(MemberAccess)

// Misc:
auto MirBuilder::visit(List* t_list) -> Any
{
  // Traverse all nodes.
  for(NodePtr& node : *t_list) {
    traverse(node);
  }

  return {};
}

// Implementation:
auto MirBuilder::get_call_args(NodeListPtr t_list) -> LocalVarVec
{
  // TODO: Check nullptr?

  LocalVarVec vec{t_list->size()};

  for(const auto& ptr : *t_list) {
    // Traverse to generate a last var that we can get.
    traverse(ptr);

    // Get the refs.
    const auto& last_var{m_factory->require_last_var()};
    vec.push_back(last_var);
  }

  return vec;
}

auto MirBuilder::translate(NodePtr t_ast) -> ModulePtr
{
  ModulePtr ptr{nullptr};

  // Initialize the module factory.
  m_factory = std::make_unique<MirModuleFactory>();

  // Traverse the AST using the module factory to create the CLIR module.
  traverse(t_ast);

  // Obtain the CLIR module created by the module factory.
  ptr = m_factory->get_module();

  // Clear the environment, as we are done with it.
  m_factory->clear_env();

  // Free the factory.
  m_factory.reset();

  return ptr;
}
} // namespace mir::mir_builder
