#include "common.h"
#include "log.h"
#include "scanner.h"
#include "tree.h"

#include <string.h>

// Fake tokens
Token false_token = {
  .kind = CONSTANT,
  .data = "false",
  .length = 5,
  .span =
    {
      .start = {-1, -1},
      .end = {-1, -1},
    },
  .next = NULL,
};

Token true_token = {
  .kind = CONSTANT,
  .data = "true",
  .length = 5,
  .span =
    {
      .start = {-1, -1},
      .end = {-1, -1},
    },
  .next = NULL,
};

void transform_translation_unit(struct translation_unit *unit);

// Entry point
void transform_ast() {
  AST tree = get_tree();
  transform_translation_unit(tree);
}

void transform_id(struct id *id) {
  UNUSED(id);
  // id->name
}

void transform_token_specifier(struct specifier *specifier) {
  UNUSED(specifier);
  // specifier->_token
}

void transform_id_specifier(struct specifier *specifier) {
  transform_id(specifier->_id);
}

void transform_specifier(struct specifier *specifier) {
  switch (specifier->type) {
  case TOKEN:
    transform_token_specifier(specifier);
    break;
  case ID_SPEC:
    transform_id_specifier(specifier);
    break;
  default:
    CRITICAL("assign", "Unknown specifier type");
  }
}

void transform_specifier_list(struct specifier_list *list) {
  for (struct specifier *cur = list->head; cur != NULL; cur = cur->next) {
    transform_specifier(cur);
  }
}

void transform_statement(struct statement *stmt);
void transform_expression(struct expression *expr);
void transform_declaration(struct declaration *decl);

void transform_initialized_declarator(struct initialized_declarator *decl) {
  transform_id(decl->declarator);

  if (decl->initializer)
    transform_expression(decl->initializer);
}

void transform_init_declarator_list(struct init_declarator_list *list) {
  if (list == NULL)
    return;

  for (struct initialized_declarator *cur = list->head; cur != NULL;
       cur = cur->next) {
    transform_initialized_declarator(cur);
  }
}

void transform_declaration_list(struct declaration_list *list) {
  struct declaration *cur = list->head;
  struct declaration *next = NULL;

  while (cur) {
    next = cur->next;
    transform_declaration(cur);
    cur = next;
  }
}

void transform_variable_definition(struct declaration *decl) {
  transform_specifier_list(decl->_var.specifiers);
  transform_init_declarator_list(decl->_var.init_declarator_list);
}

void transform_function_definition(struct declaration *decl) {
  transform_specifier_list(decl->_func.specifiers);
  transform_id(decl->_func.name);

  if (decl->_func.parameters)
    transform_declaration_list(decl->_func.parameters);

  transform_statement(decl->_func.body);
}

void transform_type_definition(struct declaration *decl) {
  transform_specifier_list(decl->_type_def.specifiers);
  transform_id(decl->_type_def.name);
}

void transform_declaration(struct declaration *decl) {
  switch (decl->type) {
  case VARIABLE:
    transform_variable_definition(decl);
    break;
  case FUNCTION:
    transform_function_definition(decl);
    break;
  case TYPEDEF:
    transform_type_definition(decl);
    break;
  default:
    CRITICAL("ast", "Unknown declaration type");
  }
}

void transform_break_stmt(struct statement *stmt) { UNUSED(stmt); }

void transform_statement_list(struct statement_list *list) {
  if (list == NULL)
    return;

  for (struct statement *child = list->head; child != NULL;
       child = child->next) {
    transform_statement(child);
  }
}

void transform_compound_stmt(struct statement *stmt) {
  transform_statement_list(stmt->_compound.statements);
}

void transform_continue_stmt(struct statement *stmt) { UNUSED(stmt); }

void transform_decl_stmt(struct statement *stmt) {
  transform_declaration(stmt->_decl);
}

void transform_expr_stmt(struct statement *stmt) {
  transform_expression(stmt->_expr);
}

void transform_for_stmt(struct statement *stmt) {
  if (stmt->_for.decl) {
    transform_declaration(stmt->_for.decl);
  }

  if (stmt->_for.preloop_expression) {
    transform_expression(stmt->_for.preloop_expression);
  }

  if (stmt->_for.condition) {
    transform_expression(stmt->_for.condition);
  }

  if (stmt->_for.step_expression) {
    transform_expression(stmt->_for.step_expression);
  }

  if (stmt->_for.body) {
    transform_statement(stmt->_for.body);
  }
}

void transform_goto_stmt(struct statement *stmt) { transform_id(stmt->_goto); }

void transform_if_stmt(struct statement *stmt) {
  if (stmt->_if.condition) {
    transform_expression(stmt->_if.condition);
  }

  if (stmt->_if.body) {
    transform_statement(stmt->_if.body);
  }

  if (stmt->_if.else_body) {
    transform_statement(stmt->_if.else_body);
  }
}

void transform_label_stmt(struct statement *stmt) {
  transform_id(stmt->_label.name);
}

void transform_return_stmt(struct statement *stmt) {
  if (stmt->_return.ret_expr) {
    transform_expression(stmt->_return.ret_expr);
  }
}

void transform_switch_stmt(struct statement *stmt) {
  if (stmt->_switch.condition) {
    transform_expression(stmt->_switch.condition);
  }

  if (stmt->_switch.body) {
    transform_statement(stmt->_switch.body);
  }
}

void transform_switch_label_stmt(struct statement *stmt) {
  if (stmt->_switch_label.test) {
    transform_expression(stmt->_switch_label.test);
  }
}

void transform_while_stmt(struct statement *stmt) {
  // stmt->_while.should_check_condition_first

  if (stmt->_while.condition) {
    transform_expression(stmt->_while.condition);
  }

  if (stmt->_while.body) {
    transform_statement(stmt->_while.body);
  }
}

void transform_statement(struct statement *stmt) {
  switch (stmt->type) {
  case BREAK:
    transform_break_stmt(stmt);
    break;
  case COMPOUND:
    transform_compound_stmt(stmt);
    break;
  case CONTINUE:
    transform_continue_stmt(stmt);
    break;
  case DECL:
    transform_decl_stmt(stmt);
    break;
  case EXPR:
    transform_expr_stmt(stmt);
    break;
  case FOR:
    transform_for_stmt(stmt);
    break;
  case GOTO:
    transform_goto_stmt(stmt);
    break;
  case IF:
    transform_if_stmt(stmt);
    break;
  case LABEL:
    transform_label_stmt(stmt);
    break;
  case RETURN:
    transform_return_stmt(stmt);
    break;
  case SWITCH:
    transform_switch_stmt(stmt);
    break;
  case SWITCH_LABEL:
    transform_switch_label_stmt(stmt);
    break;
  case WHILE:
    transform_while_stmt(stmt);
    break;
  default:
    CRITICAL("assign", "Unknown statement type");
  }
}

void transform_expression_list(struct expression_list *list);

void transform_constant_expr(struct expression *expr) {
  UNUSED(expr);
  // expr->_constant
}

void transform_id_expression(struct expression *expr) {
  transform_id(expr->_id);
}

void transform_index_expression(struct expression *expr) {
  if (expr->_index.object) {
    transform_expression(expr->_index.object);
  }

  if (expr->_index.index) {
    transform_expression(expr->_index.index);
  }
}

void transform_func_call(struct expression *expr) {
  if (expr->_call.function_ptr) {
    transform_expression(expr->_call.function_ptr);
  }

  if (expr->_call.parameter_list) {
    transform_expression_list(expr->_call.parameter_list);
  }
}

void transform_postfix_expr(struct expression *expr) {
  if (expr->_unary.base) {
    transform_expression(expr->_unary.base);
  }

  // expr->_unary.operator
}

void transform_unary_expr(struct expression *expr) {
  // expr->_unary.operator

  if (expr->_unary.base) {
    transform_expression(expr->_unary.base);
  }
}

void transform_cast_expr(struct expression *expr) {
  if (expr->_cast.type) {
    transform_specifier_list(expr->_cast.type);
  }

  if (expr->_cast.base) {
    transform_expression(expr->_cast.base);
  }
}

void transform_ternary_expr(struct expression *expr);

void short_circuit_binary_expr(struct expression *expr) {
  bool is_and = strcmp(expr->_binary.operator->data, "&&") == 0;

  struct expression ternary = {
    .type = TERNARY,
    .reg = expr->reg,
    .reg_count = expr->reg_count,
    .next = NULL,
  };

  ternary._ternary.condition = expr->_binary.left;
  ternary._ternary.true_branch =
    is_and ? expr->_binary.right : create_const_expression(&true_token);
  ternary._ternary.false_branch =
    is_and ? create_const_expression(&false_token) : expr->_binary.right;

  memcpy(expr, &ternary, sizeof(struct expression));

  transform_ternary_expr(expr);
}

void transform_binary_expr(struct expression *expr) {
  if (strcmp(expr->_binary.operator->data, "&&") == 0 ||
      strcmp(expr->_binary.operator->data, "||") == 0) {
    // Short-circuit evaluation for logical operators
    short_circuit_binary_expr(expr);
    return;
  }

  if (expr->_binary.left) {
    transform_expression(expr->_binary.left);
  }

  // expr->_binary.operator

  if (expr->_binary.right) {
    transform_expression(expr->_binary.right);
  }
}

void transform_ternary_expr(struct expression *expr) {
  if (expr->_ternary.condition) {
    transform_expression(expr->_ternary.condition);
  }

  if (expr->_ternary.true_branch) {
    transform_expression(expr->_ternary.true_branch);
  }

  if (expr->_ternary.false_branch) {
    transform_expression(expr->_ternary.false_branch);
  }
}

void transform_expression(struct expression *expr) {
  if (expr == NULL) {
    return;
  }

  switch (expr->type) {
  case CONST_EXPR:
    transform_constant_expr(expr);
    break;
  case ID_EXPR:
    transform_id_expression(expr);
    break;
  case INDEX:
    transform_index_expression(expr);
    break;
  case FUNC_CALL:
    transform_func_call(expr);
    break;
  case POSTFIX:
    transform_postfix_expr(expr);
    break;
  case UNARY:
    transform_unary_expr(expr);
    break;
  case CAST:
    transform_cast_expr(expr);
    break;
  case BINARY:
    transform_binary_expr(expr);
    break;
  case TERNARY:
    transform_ternary_expr(expr);
    break;
  default:
    CRITICAL("assign", "Unknown expression type");
  }
}

void transform_expression_list(struct expression_list *list) {
  if (list == NULL) {
    return;
  }

  struct expression *cur = list->head;

  if (cur == NULL) {
    return;
  }

  while (cur != NULL) {
    transform_expression(cur);
    cur = cur->next;
  }
}

void transform_translation_unit(struct translation_unit *unit) {
  transform_declaration_list(&unit->external_declarations);
}
