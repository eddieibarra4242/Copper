#include "common.h"
#include "log.h"
#include "tree.h"

void $$_translation_unit(struct translation_unit *unit);

// Entry point
void entry() {
  AST tree = get_tree();
  $$_translation_unit(tree);
}

void $$_id(struct id *id) {
  UNUSED(id);
  // id->name
}

void $$_token_specifier(struct specifier *specifier) {
  UNUSED(specifier);
  // specifier->_token
}

void $$_id_specifier(struct specifier *specifier) { $$_id(specifier->_id); }

void $$_specifier(struct specifier *specifier) {
  switch (specifier->type) {
  case TOKEN:
    $$_token_specifier(specifier);
    break;
  case ID_SPEC:
    $$_id_specifier(specifier);
    break;
  default:
    CRITICAL("assign", "Unknown specifier type");
  }
}

void $$_specifier_list(struct specifier_list *list) {
  for (struct specifier *cur = list->head; cur != NULL; cur = cur->next) {
    $$_specifier(cur);
  }
}

void $$_statement(struct statement *stmt);
void $$_expression(struct expression *expr);
void $$_declaration(struct declaration *decl);

void $$_initialized_declarator(struct initialized_declarator *decl) {
  $$_id(decl->declarator);

  if (decl->initializer)
    $$_expression(decl->initializer);
}

void $$_init_declarator_list(struct init_declarator_list *list) {
  if (list == NULL)
    return;

  for (struct initialized_declarator *cur = list->head; cur != NULL;
       cur = cur->next) {
    $$_initialized_declarator(cur);
  }
}

void $$_declaration_list(struct declaration_list *list) {
  struct declaration *cur = list->head;
  struct declaration *next = NULL;

  while (cur) {
    next = cur->next;
    $$_declaration(cur);
    cur = next;
  }
}

void $$_variable_definition(struct declaration *decl) {
  $$_specifier_list(decl->_var.specifiers);
  $$_init_declarator_list(decl->_var.init_declarator_list);
}

void $$_function_definition(struct declaration *decl) {
  $$_specifier_list(decl->_func.specifiers);
  $$_id(decl->_func.name);

  if (decl->_func.parameters)
    $$_declaration_list(decl->_func.parameters);
  
  $$_statement(decl->_func.body);
}

void $$_type_definition(struct declaration *decl) {
  $$_specifier_list(decl->_type_def.specifiers);
  $$_id(decl->_type_def.name);
}

void $$_declaration(struct declaration *decl) {
  switch (decl->type) {
  case VARIABLE:
    $$_variable_definition(decl);
    break;
  case FUNCTION:
    $$_function_definition(decl);
    break;
  case TYPEDEF:
    $$_type_definition(decl);
    break;
  default:
    CRITICAL("ast", "Unknown declaration type");
  }
}

void $$_break_stmt(struct statement *stmt) { UNUSED(stmt); }

void $$_statement_list(struct statement_list *list) {
  if (list == NULL)
    return;

  for (struct statement *child = list->head; child != NULL;
       child = child->next) {
    $$_statement(child);
  }
}

void $$_compound_stmt(struct statement *stmt) {
  $$_statement_list(stmt->_compound.statements);
}

void $$_continue_stmt(struct statement *stmt) { UNUSED(stmt); }

void $$_decl_stmt(struct statement *stmt) { $$_declaration(stmt->_decl); }

void $$_expr_stmt(struct statement *stmt) { $$_expression(stmt->_expr); }

void $$_for_stmt(struct statement *stmt) {
  if (stmt->_for.decl) {
    $$_declaration(stmt->_for.decl);
  }

  if (stmt->_for.preloop_expression) {
    $$_expression(stmt->_for.preloop_expression);
  }

  if (stmt->_for.condition) {
    $$_expression(stmt->_for.condition);
  }

  if (stmt->_for.step_expression) {
    $$_expression(stmt->_for.step_expression);
  }

  if (stmt->_for.body) {
    $$_statement(stmt->_for.body);
  }
}

void $$_goto_stmt(struct statement *stmt) { $$_id(stmt->_goto); }

void $$_if_stmt(struct statement *stmt) {
  if (stmt->_if.condition) {
    $$_expression(stmt->_if.condition);
  }

  if (stmt->_if.body) {
    $$_statement(stmt->_if.body);
  }

  if (stmt->_if.else_body) {
    $$_statement(stmt->_if.else_body);
  }
}

void $$_label_stmt(struct statement *stmt) { $$_id(stmt->_label.name); }

void $$_return_stmt(struct statement *stmt) {
  if (stmt->_return.ret_expr) {
    $$_expression(stmt->_return.ret_expr);
  }
}

void $$_switch_stmt(struct statement *stmt) {
  if (stmt->_switch.condition) {
    $$_expression(stmt->_switch.condition);
  }

  if (stmt->_switch.body) {
    $$_statement(stmt->_switch.body);
  }
}

void $$_switch_label_stmt(struct statement *stmt) {
  if (stmt->_switch_label.test) {
    $$_expression(stmt->_switch_label.test);
  }
}

void $$_while_stmt(struct statement *stmt) {
  // stmt->_while.should_check_condition_first

  if (stmt->_while.condition) {
    $$_expression(stmt->_while.condition);
  }

  if (stmt->_while.body) {
    $$_statement(stmt->_while.body);
  }
}

void $$_statement(struct statement *stmt) {
  switch (stmt->type) {
  case BREAK:
    $$_break_stmt(stmt);
    break;
  case COMPOUND:
    $$_compound_stmt(stmt);
    break;
  case CONTINUE:
    $$_continue_stmt(stmt);
    break;
  case DECL:
    $$_decl_stmt(stmt);
    break;
  case EXPR:
    $$_expr_stmt(stmt);
    break;
  case FOR:
    $$_for_stmt(stmt);
    break;
  case GOTO:
    $$_goto_stmt(stmt);
    break;
  case IF:
    $$_if_stmt(stmt);
    break;
  case LABEL:
    $$_label_stmt(stmt);
    break;
  case RETURN:
    $$_return_stmt(stmt);
    break;
  case SWITCH:
    $$_switch_stmt(stmt);
    break;
  case SWITCH_LABEL:
    $$_switch_label_stmt(stmt);
    break;
  case WHILE:
    $$_while_stmt(stmt);
    break;
  default:
    CRITICAL("assign", "Unknown statement type");
  }
}

void $$_expression_list(struct expression_list *list);

void $$_constant_expr(struct expression *expr) {
  UNUSED(expr);
  // expr->_constant
}

void $$_id_expression(struct expression *expr) { $$_id(expr->_id); }

void $$_index_expression(struct expression *expr) {
  if (expr->_index.object) {
    $$_expression(expr->_index.object);
  }

  if (expr->_index.index) {
    $$_expression(expr->_index.index);
  }
}

void $$_func_call(struct expression *expr) {
  if (expr->_call.function_ptr) {
    $$_expression(expr->_call.function_ptr);
  }

  if (expr->_call.parameter_list) {
    $$_expression_list(expr->_call.parameter_list);
  }
}

void $$_postfix_expr(struct expression *expr) {
  if (expr->_unary.base) {
    $$_expression(expr->_unary.base);
  }

  // expr->_unary.operator
}

void $$_unary_expr(struct expression *expr) {
  // expr->_unary.operator

  if (expr->_unary.base) {
    $$_expression(expr->_unary.base);
  }
}

void $$_cast_expr(struct expression *expr) {
  if (expr->_cast.type) {
    $$_specifier_list(expr->_cast.type);
  }

  if (expr->_cast.base) {
    $$_expression(expr->_cast.base);
  }
}

void $$_binary_expr(struct expression *expr) {
  if (expr->_binary.left) {
    $$_expression(expr->_binary.left);
  }

  // expr->_binary.operator

  if (expr->_binary.right) {
    $$_expression(expr->_binary.right);
  }
}

void $$_ternary_expr(struct expression *expr) {
  if (expr->_ternary.condition) {
    $$_expression(expr->_ternary.condition);
  }

  if (expr->_ternary.true_branch) {
    $$_expression(expr->_ternary.true_branch);
  }

  if (expr->_ternary.false_branch) {
    $$_expression(expr->_ternary.false_branch);
  }
}

void $$_expression(struct expression *expr) {
  if (expr == NULL) {
    return;
  }

  switch (expr->type) {
  case CONST_EXPR:
    $$_constant_expr(expr);
    break;
  case ID_EXPR:
    $$_id_expression(expr);
    break;
  case INDEX:
    $$_index_expression(expr);
    break;
  case FUNC_CALL:
    $$_func_call(expr);
    break;
  case POSTFIX:
    $$_postfix_expr(expr);
    break;
  case UNARY:
    $$_unary_expr(expr);
    break;
  case CAST:
    $$_cast_expr(expr);
    break;
  case BINARY:
    $$_binary_expr(expr);
    break;
  case TERNARY:
    $$_ternary_expr(expr);
    break;
  default:
    CRITICAL("assign", "Unknown expression type");
  }
}

void $$_expression_list(struct expression_list *list) {
  if (list == NULL) {
    return;
  }

  struct expression *cur = list->head;

  if (cur == NULL) {
    return;
  }

  while (cur != NULL) {
    $$_expression(cur);
    cur = cur->next;
  }
}

void $$_translation_unit(struct translation_unit *unit) {
  $$_declaration_list(&unit->external_declarations);
}
