#include "emit.h"

#include "common.h"
#include "log.h"
#include "tree.h"
#include "utils.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define APPEND_LIST(list, child_function)                                      \
  do {                                                                         \
    InstructionList *__child_list = (child_function);                          \
    if (__child_list == NULL) {                                                \
      CRITICALV("emit", "Child list is NULL at %s:%d", __FILE__, __LINE__);    \
    }                                                                          \
    append_instruction_list((list), __child_list);                             \
    free(__child_list);                                                        \
  } while (0)

struct GenLabel {
  char name[64];
  struct GenLabel *next;
};

InstructionList *emit_statement(struct statement *stmt);
InstructionList *emit_lval_expression(struct expression *expr);
InstructionList *emit_rval_expression(struct expression *expr);

/**
 * Emits an expression that uses both l-value and r-value semantics.
 * This is typically used for assignments and other expressions that require
 * both reading and writing to a variable.
 *
 * @param expr The expression to emit.
 * @return A list of instructions representing the emitted expression.
 */
InstructionList *emit_expression(struct expression *expr);

struct GenLabel *gen_label_list = NULL;

const char *generate_label(const char *prefix) {
  const char *label_prefix = (prefix != NULL) ? prefix : "L";
  static uint64_t label_counter = 0;
  struct GenLabel *new_label = malloc(sizeof(struct GenLabel));
  if (new_label == NULL) {
    CRITICAL("emit", "Out of memory!");
  }

  snprintf(new_label->name, sizeof(new_label->name), ".%s_%lu", label_prefix,
           label_counter++);

  new_label->next = gen_label_list;
  gen_label_list = new_label;
  return new_label->name;
}

void free_generated_labels(void) {
  struct GenLabel *cur = gen_label_list;
  while (cur != NULL) {
    struct GenLabel *next = cur->next;
    free(cur);
    cur = next;
  }
  gen_label_list = NULL;
}

InstructionList *emit_control_flow(struct expression *condition, bool eval,
                                   const char *label) {
  InstructionList *insns = emit_rval_expression(condition);

  enum InstructionSet opcode = eval ? JUMP_NOT_ZERO : JUMP_EQU_ZERO;
  append_instruction(insns, opcode, OP_LABEL(label), OP_NONE, OP_NONE);

  return insns;
}

InstructionList *emit_translation_unit(struct translation_unit *unit);
InstructionList *emit_declaration(struct declaration *decl);

// Entry point
InstructionList *emit_intermediate_representation() {
  AST tree = get_tree();
  InstructionList *list = emit_translation_unit(tree);
  return list;
}

void emit_id(struct id *id) {
  UNUSED(id);
  // id->name
}

void emit_token_specifier(struct specifier *specifier) {
  UNUSED(specifier);
  // specifier->_token
}

void emit_id_specifier(struct specifier *specifier) { emit_id(specifier->_id); }

void emit_specifier(struct specifier *specifier) {
  switch (specifier->type) {
  case TOKEN:
    emit_token_specifier(specifier);
    break;
  case ID_SPEC:
    emit_id_specifier(specifier);
    break;
  default:
    CRITICAL("assign", "Unknown specifier type");
  }
}

void emit_specifier_list(struct specifier_list *list) {
  for (struct specifier *cur = list->head; cur != NULL; cur = cur->next) {
    emit_specifier(cur);
  }
}

void emit_initialized_declarator(struct initialized_declarator *decl) {
  emit_id(decl->declarator);

  if (decl->initializer)
    emit_expression(decl->initializer);
}

void emit_init_declarator_list(struct init_declarator_list *list) {
  if (list == NULL)
    return;

  for (struct initialized_declarator *cur = list->head; cur != NULL;
       cur = cur->next) {
    emit_initialized_declarator(cur);
  }
}

InstructionList *emit_declaration_list(struct declaration_list *list) {
  InstructionList *insns = create_instruction_list();

  for (struct declaration *cur = list->head; cur != NULL; cur = cur->next) {
    APPEND_LIST(insns, emit_declaration(cur));
  }

  return insns;
}

void emit_variable_definition(struct declaration *decl) {
  emit_specifier_list(decl->_var.specifiers);
  emit_init_declarator_list(decl->_var.init_declarator_list);
}

InstructionList *emit_function(struct declaration *decl) {
  InstructionList *insns = create_instruction_list();

  // Emit function label
  if (decl->_func.name) {
    append_instruction(insns, ILABEL, OP_LABEL(decl->_func.name->name->data),
                       OP_NONE, OP_NONE);
  } else {
    CRITICAL("emit", "Function declaration without a name");
  }

  // Emit function prologue

  // Emit function body
  if (decl->_func.body) {
    APPEND_LIST(insns, emit_statement(decl->_func.body));
  } else {
    // empty function body
    // This is a no-op, but we still need to emit an instruction to return for
    // function calls.
    append_instruction(insns, IRETURN, OP_NONE, OP_NONE, OP_NONE);
  }

  // Emit function epilogue

  return insns; // FIXME
}

void emit_type_definition(struct declaration *decl) {
  emit_specifier_list(decl->_type_def.specifiers);
  emit_id(decl->_type_def.name);
}

InstructionList *emit_declaration(struct declaration *decl) {
  switch (decl->type) {
  case VARIABLE:
    emit_variable_definition(decl);
    break;
  case FUNCTION:
    return emit_function(decl);
  case TYPEDEF:
    emit_type_definition(decl);
    break;
  default:
    CRITICAL("ast", "Unknown declaration type");
  }

  return create_instruction_list();
}

void emit_break_stmt(struct statement *stmt) { UNUSED(stmt); }

InstructionList *emit_statement_list(struct statement_list *list) {
  InstructionList *insns = create_instruction_list();

  if (list == NULL)
    return insns;

  for (struct statement *child = list->head; child != NULL;
       child = child->next) {
    APPEND_LIST(insns, emit_statement(child));
  }

  return insns;
}

InstructionList *emit_compound_stmt(struct statement *stmt) {
  return emit_statement_list(stmt->_compound.statements);
}

void emit_continue_stmt(struct statement *stmt) { UNUSED(stmt); }

void emit_decl_stmt(struct statement *stmt) { emit_declaration(stmt->_decl); }

void emit_expr_stmt(struct statement *stmt) { emit_expression(stmt->_expr); }

void emit_for_stmt(struct statement *stmt) {
  if (stmt->_for.decl) {
    emit_declaration(stmt->_for.decl);
  }

  if (stmt->_for.preloop_expression) {
    emit_expression(stmt->_for.preloop_expression);
  }

  if (stmt->_for.condition) {
    emit_rval_expression(stmt->_for.condition);
  }

  if (stmt->_for.step_expression) {
    emit_expression(stmt->_for.step_expression);
  }

  if (stmt->_for.body) {
    emit_statement(stmt->_for.body);
  }
}

void emit_goto_stmt(struct statement *stmt) { emit_id(stmt->_goto); }

InstructionList *emit_if_stmt(struct statement *stmt) {
  const char *label_else = generate_label("else");
  const char *label_end = generate_label("end_if");

  bool has_else = stmt->_if.else_body != NULL;

  InstructionList *insns = emit_control_flow(stmt->_if.condition, false,
                                             has_else ? label_else : label_end);

  APPEND_LIST(insns, emit_statement(stmt->_if.body));

  if (has_else) {
    append_instruction(insns, JUMP, OP_LABEL(label_end), OP_NONE, OP_NONE);
    append_instruction(insns, ILABEL, OP_LABEL(label_else), OP_NONE, OP_NONE);
    APPEND_LIST(insns, emit_statement(stmt->_if.else_body));
  }

  append_instruction(insns, ILABEL, OP_LABEL(label_end), OP_NONE, OP_NONE);

  return insns;
}

void emit_label_stmt(struct statement *stmt) { emit_id(stmt->_label.name); }

InstructionList *emit_return_stmt(struct statement *stmt) {
  InstructionList *insns = create_instruction_list();

  if (stmt->_return.ret_expr) {
    APPEND_LIST(insns, emit_rval_expression(stmt->_return.ret_expr));
  }

  append_instruction(insns, IRETURN, OP_NONE, OP_NONE, OP_NONE);
  return insns;
}

void emit_switch_stmt(struct statement *stmt) {
  if (stmt->_switch.condition) {
    emit_rval_expression(stmt->_switch.condition);
  }

  if (stmt->_switch.body) {
    emit_statement(stmt->_switch.body);
  }
}

void emit_switch_label_stmt(struct statement *stmt) {
  if (stmt->_switch_label.test) {
    emit_expression(stmt->_switch_label.test);
  }
}

void emit_while_stmt(struct statement *stmt) {
  // stmt->_while.should_check_condition_first

  if (stmt->_while.condition) {
    emit_rval_expression(stmt->_while.condition);
  }

  if (stmt->_while.body) {
    emit_statement(stmt->_while.body);
  }
}

InstructionList *emit_statement(struct statement *stmt) {
  switch (stmt->type) {
  case BREAK:
    emit_break_stmt(stmt);
    break;
  case COMPOUND:
    return emit_compound_stmt(stmt);
  case CONTINUE:
    emit_continue_stmt(stmt);
    break;
  case DECL:
    emit_decl_stmt(stmt);
    break;
  case EXPR:
    emit_expr_stmt(stmt);
    break;
  case FOR:
    emit_for_stmt(stmt);
    break;
  case GOTO:
    emit_goto_stmt(stmt);
    break;
  case IF:
    return emit_if_stmt(stmt);
  case LABEL:
    emit_label_stmt(stmt);
    break;
  case RETURN:
    return emit_return_stmt(stmt);
  case SWITCH:
    emit_switch_stmt(stmt);
    break;
  case SWITCH_LABEL:
    emit_switch_label_stmt(stmt);
    break;
  case WHILE:
    emit_while_stmt(stmt);
    break;
  default:
    CRITICAL("assign", "Unknown statement type");
  }

  // If the statement does not generate any instructions, return an empty list.
  return create_instruction_list();
}

void emit_rval_expression_list(struct expression_list *list);

InstructionList *emit_rval_constant_expr(struct expression *expr) {
  InstructionList *insns = create_instruction_list();
  Constant value = eval_token(expr->_constant);

  append_instruction(insns, LOAD_CONST, OP_REG(expr->reg), OP_CONST(value.bits),
                     OP_NONE);

  return insns;
}

void emit_rval_id_expression(struct expression *expr) { emit_id(expr->_id); }

void emit_rval_index_expression(struct expression *expr) {
  if (expr->_index.object) {
    emit_rval_expression(expr->_index.object);
  }

  if (expr->_index.index) {
    emit_rval_expression(expr->_index.index);
  }
}

void emit_rval_func_call(struct expression *expr) {
  if (expr->_call.function_ptr) {
    emit_rval_expression(expr->_call.function_ptr);
  }

  if (expr->_call.parameter_list) {
    emit_rval_expression_list(expr->_call.parameter_list);
  }
}

void emit_rval_postfix_expr(struct expression *expr) {
  if (expr->_unary.base) {
    emit_rval_expression(expr->_unary.base);
  }

  // expr->_unary.operator
}

InstructionList *emit_rval_unary_expr(struct expression *expr) {
  InstructionList *insns = emit_rval_expression(expr->_unary.base);

  enum InstructionSet opcode = 0;
  Operand operand_second = OP_NONE;

  if (strcmp(expr->_unary.operator->data, "!") == 0) {
    opcode = LOGICAL_NOT;
  } else if (strcmp(expr->_unary.operator->data, "~") == 0) {
    opcode = BITWISE_NOT;
  } else if (strcmp(expr->_unary.operator->data, "-") == 0) {
    // FIXME: could be better. bitwise not and add 1.
    opcode = MULTIPLY;
    operand_second = OP_CONST(-1);
  } else if (strcmp(expr->_unary.operator->data, "+") == 0) {
    return insns;
  } else {
    CRITICALV("emit", "Unknown unary operator: %s",
              expr->_unary.operator->data);
  }

  append_instruction(insns, opcode, OP_REG(expr->reg),
                     OP_REG(expr->_unary.base->reg), operand_second);
  return insns;
}

void emit_rval_cast_expr(struct expression *expr) {
  if (expr->_cast.type) {
    emit_specifier_list(expr->_cast.type);
  }

  if (expr->_cast.base) {
    emit_rval_expression(expr->_cast.base);
  }
}

InstructionList *emit_rval_binary_expr(struct expression *expr) {
  struct expression *first =
      expr->_binary.left->reg_count > expr->_binary.right->reg_count
          ? expr->_binary.left
          : expr->_binary.right;
  struct expression *second =
      expr->_binary.left->reg_count > expr->_binary.right->reg_count
          ? expr->_binary.right
          : expr->_binary.left;

  InstructionList *insns = emit_rval_expression(first);
  APPEND_LIST(insns, emit_rval_expression(second));

  enum InstructionSet opcode = 0;

  if (strcmp(expr->_binary.operator->data, "||") == 0) {
    opcode = LOGICAL_OR;
  } else if (strcmp(expr->_binary.operator->data, "&&") == 0) {
    opcode = LOGICAL_AND;
  } else if (strcmp(expr->_binary.operator->data, "|") == 0) {
    opcode = BITWISE_OR;
  } else if (strcmp(expr->_binary.operator->data, "^") == 0) {
    opcode = BITWISE_XOR;
  } else if (strcmp(expr->_binary.operator->data, "&") == 0) {
    opcode = BITWISE_AND;
  } else if (strcmp(expr->_binary.operator->data, "==") == 0) {
    opcode = EQUAL;
  } else if (strcmp(expr->_binary.operator->data, "!=") == 0) {
    opcode = NOT_EQUAL;
  } else if (strcmp(expr->_binary.operator->data, ">=") == 0) {
    opcode = GREATER_EQUAL;
  } else if (strcmp(expr->_binary.operator->data, "<=") == 0) {
    opcode = LESS_EQUAL;
  } else if (strcmp(expr->_binary.operator->data, ">") == 0) {
    opcode = GREATER_THAN;
  } else if (strcmp(expr->_binary.operator->data, "<") == 0) {
    opcode = LESS_THAN;
  } else if (strcmp(expr->_binary.operator->data, ">>") == 0) {
    opcode = RIGHT_SHIFT;
  } else if (strcmp(expr->_binary.operator->data, "<<") == 0) {
    opcode = LEFT_SHIFT;
  } else if (strcmp(expr->_binary.operator->data, "-") == 0) {
    opcode = SUBTRACT;
  } else if (strcmp(expr->_binary.operator->data, "+") == 0) {
    opcode = ADD;
  } else if (strcmp(expr->_binary.operator->data, "%") == 0) {
    opcode = MODULO;
  } else if (strcmp(expr->_binary.operator->data, "/") == 0) {
    opcode = DIVIDE;
  } else if (strcmp(expr->_binary.operator->data, "*") == 0) {
    opcode = MULTIPLY;
  } else {
    CRITICALV("emit", "Unknown binary operator: %s",
              expr->_binary.operator->data);
  }

  append_instruction(insns, opcode, OP_REG(expr->reg),
                     OP_REG(expr->_binary.left->reg),
                     OP_REG(expr->_binary.right->reg));

  return insns;
}

InstructionList *emit_rval_ternary_expr(struct expression *expr) {
  const char *false_label = generate_label("ternary_false");
  const char *end_label = generate_label("ternary_end");

  InstructionList *insns =
      emit_control_flow(expr->_ternary.condition, false, false_label);

  APPEND_LIST(insns, emit_rval_expression(expr->_ternary.true_branch));
  append_instruction(insns, JUMP, OP_LABEL(end_label), OP_NONE, OP_NONE);
  append_instruction(insns, ILABEL, OP_LABEL(false_label), OP_NONE, OP_NONE);
  APPEND_LIST(insns, emit_rval_expression(expr->_ternary.false_branch));
  append_instruction(insns, ILABEL, OP_LABEL(end_label), OP_NONE, OP_NONE);

  return insns;
}

InstructionList *emit_rval_expression(struct expression *expr) {
  if (expr == NULL) {
    return create_instruction_list();
  }

  switch (expr->type) {
  case CONST_EXPR:
    return emit_rval_constant_expr(expr);
  case ID_EXPR:
    emit_rval_id_expression(expr);
    break;
  case INDEX:
    emit_rval_index_expression(expr);
    break;
  case FUNC_CALL:
    emit_rval_func_call(expr);
    break;
  case POSTFIX:
    emit_rval_postfix_expr(expr);
    break;
  case UNARY:
    return emit_rval_unary_expr(expr);
  case CAST:
    emit_rval_cast_expr(expr);
    break;
  case BINARY:
    return emit_rval_binary_expr(expr);
  case TERNARY:
    return emit_rval_ternary_expr(expr);
  default:
    CRITICAL("assign", "Unknown expression type");
  }

  return create_instruction_list();
}

void emit_rval_expression_list(struct expression_list *list) {
  if (list == NULL) {
    return;
  }

  struct expression *cur = list->head;

  if (cur == NULL) {
    return;
  }

  while (cur != NULL) {
    emit_rval_expression(cur);
    cur = cur->next;
  }
}

void emit_lval_expression_list(struct expression_list *list);

void emit_lval_constant_expr(struct expression *expr) {
  UNUSED(expr);
  // expr->_constant

  ERROR("emit", "L-value for constant expression is not supported");
}

void emit_lval_id_expression(struct expression *expr) { emit_id(expr->_id); }

void emit_lval_index_expression(struct expression *expr) {
  if (expr->_index.object) {
    emit_lval_expression(expr->_index.object);
  }

  if (expr->_index.index) {
    emit_lval_expression(expr->_index.index);
  }
}

void emit_lval_func_call(struct expression *expr) {
  if (expr->_call.function_ptr) {
    emit_lval_expression(expr->_call.function_ptr);
  }

  if (expr->_call.parameter_list) {
    emit_lval_expression_list(expr->_call.parameter_list);
  }
}

void emit_lval_postfix_expr(struct expression *expr) {
  if (expr->_unary.base) {
    emit_lval_expression(expr->_unary.base);
  }

  // expr->_unary.operator
}

void emit_lval_unary_expr(struct expression *expr) {
  // expr->_unary.operator

  if (expr->_unary.base) {
    emit_lval_expression(expr->_unary.base);
  }
}

void emit_lval_cast_expr(struct expression *expr) {
  if (expr->_cast.type) {
    emit_specifier_list(expr->_cast.type);
  }

  if (expr->_cast.base) {
    emit_lval_expression(expr->_cast.base);
  }
}

void emit_lval_binary_expr(struct expression *expr) {
  if (expr->_binary.left) {
    emit_lval_expression(expr->_binary.left);
  }

  // expr->_binary.operator

  if (expr->_binary.right) {
    emit_lval_expression(expr->_binary.right);
  }
}

void emit_lval_ternary_expr(struct expression *expr) {
  if (expr->_ternary.condition) {
    emit_lval_expression(expr->_ternary.condition);
  }

  if (expr->_ternary.true_branch) {
    emit_lval_expression(expr->_ternary.true_branch);
  }

  if (expr->_ternary.false_branch) {
    emit_lval_expression(expr->_ternary.false_branch);
  }
}

InstructionList *emit_lval_expression(struct expression *expr) {
  if (expr == NULL) {
    return create_instruction_list();
  }

  switch (expr->type) {
  case CONST_EXPR:
    emit_lval_constant_expr(expr);
    break;
  case ID_EXPR:
    emit_lval_id_expression(expr);
    break;
  case INDEX:
    emit_lval_index_expression(expr);
    break;
  case FUNC_CALL:
    emit_lval_func_call(expr);
    break;
  case POSTFIX:
    emit_lval_postfix_expr(expr);
    break;
  case UNARY:
    emit_lval_unary_expr(expr);
    break;
  case CAST:
    emit_lval_cast_expr(expr);
    break;
  case BINARY:
    emit_lval_binary_expr(expr);
    break;
  case TERNARY:
    emit_lval_ternary_expr(expr);
    break;
  default:
    CRITICAL("assign", "Unknown expression type");
  }

  return create_instruction_list();
}

void emit_lval_expression_list(struct expression_list *list) {
  if (list == NULL) {
    return;
  }

  struct expression *cur = list->head;

  if (cur == NULL) {
    return;
  }

  while (cur != NULL) {
    emit_lval_expression(cur);
    cur = cur->next;
  }
}

InstructionList *emit_expression(struct expression *expr) {
  UNUSED(expr);
  // TODO: Implement this function to handle both l-value and r-value
  // expressions.
  return create_instruction_list();
}

InstructionList *emit_translation_unit(struct translation_unit *unit) {
  return emit_declaration_list(&unit->external_declarations);
}
