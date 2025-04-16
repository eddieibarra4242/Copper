#include "common.h"
#include "log.h"
#include "tree.h"

void translation_unit(struct translation_unit *unit);

// Entry point
void entry() {
  AST tree = get_tree();
  translation_unit(tree);
}

void id(struct id *id) {
  UNUSED(id);
  // id->name
}

void token_specifier(struct specifier *specifier) {
  UNUSED(specifier);
  // specifier->_token
}

void id_specifier(struct specifier *specifier) { id(specifier->_id); }

void specifier(struct specifier *specifier) {
  switch (specifier->type) {
  case TOKEN:
    token_specifier(specifier);
    break;
  case ID_SPEC:
    id_specifier(specifier);
    break;
  default:
    CRITICAL("assign", "Unknown specifier type");
  }
}

void specifier_list(struct specifier_list *list) {
  for (struct specifier *cur = list->head; cur != NULL; cur = cur->next) {
    specifier(cur);
  }
}

void statement(struct statement *stmt);
void expression(struct expression *expr);

void initialized_declarator(struct initialized_declarator *decl) {
  id(decl->declarator);

  if (decl->initializer)
    expression(decl->initializer);
}

void init_declarator_list(struct init_declarator_list *list) {
  for (struct initialized_declarator *cur = list->head; cur != NULL;
       cur = cur->next) {
    initialized_declarator(cur);
  }
}

void declaration(struct declaration *decl) {
  if (decl->specifiers)
    specifier_list(decl->specifiers);

  if (decl->name)
    id(decl->name);

  if (decl->body) {
    statement(decl->body);
  }

  if (decl->init_declarator_list) {
    init_declarator_list(decl->init_declarator_list);
  }
}

void declaration_list(struct declaration_list *list) {
  for (struct declaration *cur = list->head; cur != NULL; cur = cur->next) {
    declaration(cur);
  }
}

void break_stmt(struct statement *stmt) { UNUSED(stmt); }

void compound_stmt(struct statement *stmt) {
  for (struct statement *child = stmt->_compound.head; child != NULL;
       child = child->next) {
    statement(child);
  }
}

void continue_stmt(struct statement *stmt) { UNUSED(stmt); }

void decl_stmt(struct statement *stmt) { declaration(stmt->_decl); }

void expr_stmt(struct statement *stmt) { expression(stmt->_expr); }

void for_stmt(struct statement *stmt) {
  if (stmt->_for.decl) {
    declaration(stmt->_for.decl);
  }

  if (stmt->_for.preloop_expression) {
    expression(stmt->_for.preloop_expression);
  }

  if (stmt->_for.condition) {
    expression(stmt->_for.condition);
  }

  if (stmt->_for.step_expression) {
    expression(stmt->_for.step_expression);
  }

  if (stmt->_for.body) {
    statement(stmt->_for.body);
  }
}

void goto_stmt(struct statement *stmt) { id(stmt->_goto); }

void if_stmt(struct statement *stmt) {
  if (stmt->_if.condition) {
    expression(stmt->_if.condition);
  }

  if (stmt->_if.body) {
    statement(stmt->_if.body);
  }

  if (stmt->_if.else_body) {
    statement(stmt->_if.else_body);
  }
}

void label_stmt(struct statement *stmt) { id(stmt->_label.name); }

void return_stmt(struct statement *stmt) {
  if (stmt->_return.ret_expr) {
    expression(stmt->_return.ret_expr);
  }
}

void switch_stmt(struct statement *stmt) {
  if (stmt->_switch.condition) {
    expression(stmt->_switch.condition);
  }

  if (stmt->_switch.body) {
    statement(stmt->_switch.body);
  }
}

void switch_label_stmt(struct statement *stmt) {
  if (stmt->_switch_label.test) {
    expression(stmt->_switch_label.test);
  }
}

void while_stmt(struct statement *stmt) {
  // stmt->_while.should_check_condition_first

  if (stmt->_while.condition) {
    expression(stmt->_while.condition);
  }

  if (stmt->_while.body) {
    statement(stmt->_while.body);
  }
}

void statement(struct statement *stmt) {
  switch (stmt->type) {
  case BREAK:
    break_stmt(stmt);
    break;
  case COMPOUND:
    compound_stmt(stmt);
    break;
  case CONTINUE:
    continue_stmt(stmt);
    break;
  case DECL:
    decl_stmt(stmt);
    break;
  case EXPR:
    expr_stmt(stmt);
    break;
  case FOR:
    for_stmt(stmt);
    break;
  case GOTO:
    goto_stmt(stmt);
    break;
  case IF:
    if_stmt(stmt);
    break;
  case LABEL:
    label_stmt(stmt);
    break;
  case RETURN:
    return_stmt(stmt);
    break;
  case SWITCH:
    switch_stmt(stmt);
    break;
  case SWITCH_LABEL:
    switch_label_stmt(stmt);
    break;
  case WHILE:
    while_stmt(stmt);
    break;
  default:
    CRITICAL("assign", "Unknown statement type");
  }
}

void expression_list(struct expression_list *list);

void constant_expr(struct expression *expr) {
  UNUSED(expr);
  // expr->_constant
}

void id_expression(struct expression *expr) {
  UNUSED(expr);
  // expr->_id
}

void index_expression(struct expression *expr) {
  if (expr->_index.object) {
    expression(expr->_index.object);
  }

  if (expr->_index.index) {
    expression(expr->_index.index);
  }
}

void func_call(struct expression *expr) {
  if (expr->_call.function_ptr) {
    expression(expr->_call.function_ptr);
  }

  if (expr->_call.parameter_list) {
    expression_list(expr->_call.parameter_list);
  }
}

void postfix_expr(struct expression *expr) {
  if (expr->_unary.base) {
    expression(expr->_unary.base);
  }

  // expr->_unary.operator
}

void unary_expr(struct expression *expr) {
  // expr->_unary.operator

  if (expr->_unary.base) {
    expression(expr->_unary.base);
  }
}

void cast_expr(struct expression *expr) {
  if (expr->_cast.type) {
    specifier_list(expr->_cast.type);
  }

  if (expr->_cast.base) {
    expression(expr->_cast.base);
  }
}

void binary_expr(struct expression *expr) {
  if (expr->_binary.left) {
    expression(expr->_binary.left);
  }

  // expr->_binary.operator

  if (expr->_binary.right) {
    expression(expr->_binary.right);
  }
}

void ternay_expr(struct expression *expr) {
  if (expr->_ternary.condition) {
    expression(expr->_ternary.condition);
  }

  if (expr->_ternary.true_branch) {
    expression(expr->_ternary.true_branch);
  }

  if (expr->_ternary.false_branch) {
    expression(expr->_ternary.false_branch);
  }
}

void expression(struct expression *expr) {
  if (expr == NULL) {
    return;
  }

  switch (expr->type) {
  case CONST_EXPR:
    constant_expr(expr);
    break;
  case ID_EXPR:
    id_expression(expr);
    break;
  case INDEX:
    index_expression(expr);
    break;
  case FUNC_CALL:
    func_call(expr);
    break;
  case POSTFIX:
    postfix_expr(expr);
    break;
  case UNARY:
    unary_expr(expr);
    break;
  case CAST:
    cast_expr(expr);
    break;
  case BINARY:
    binary_expr(expr);
    break;
  case TERNARY:
    ternay_expr(expr);
    break;
  default:
    CRITICAL("assign", "Unknown expression type");
  }
}

void expression_list(struct expression_list *list) {
  if (list == NULL) {
    return;
  }

  struct expression *cur = list->head;

  if (cur == NULL) {
    return;
  }

  while (cur != NULL) {
    expression(cur);
    cur = cur->next;
  }
}

void translation_unit(struct translation_unit *unit) {
  declaration_list(&unit->external_declarations);
}
