#include "debug_ast.h"
#include "common.h"
#include "log.h"
#include "tree.h"

#include <stdarg.h>
#include <stdio.h>

void print_translation_unit(struct translation_unit *unit);

// Entry point
void print_ast() {
  AST tree = get_tree();
  print_translation_unit(tree);
}

size_t stack = 0;

char *get_prefix() {
  size_t length = 2 * stack + 1;
  char *prefix = malloc(length);

  if (prefix == NULL) {
    CRITICAL("print ast", "Out of memory!");
  }

  for (size_t i = 0; i < stack; i++) {
    prefix[2 * i] = '|';
    prefix[2 * i + 1] = ' ';
  }

  if (length >= 3) {
    prefix[length - 3] = '+';
    prefix[length - 2] = '-';
  }

  prefix[length - 1] = '\0';

  return prefix;
}

void print(const char *fmt, ...) {
  char fmtted_string[512];
  char *prefix = get_prefix();

  va_list args;
  va_start(args, fmt);
  vsnprintf(fmtted_string, 512, fmt, args);
  va_end(args);

  DEBUG("%s%s", prefix, fmtted_string);

  free(prefix);
}

void print_token(Token *token) { print("Token { %s }", token->data); }

void print_id(struct id *id) {
  print("Id");

  stack++;
  print_token(id->name);
  print("Symbol: %p", id->symbol);
  stack--;
}

void print_token_specifier(struct specifier *specifier) {
  print("Token specifier");

  stack++;
  print_token(specifier->_token);
  stack--;
}

void print_id_specifier(struct specifier *specifier) {
  print("Id specifier");

  stack++;
  print_id(specifier->_id);
  stack--;
}

void print_specifier(struct specifier *specifier) {
  switch (specifier->type) {
  case TOKEN:
    print_token_specifier(specifier);
    break;
  case ID_SPEC:
    print_id_specifier(specifier);
    break;
  }
}

void print_specifier_list(struct specifier_list *list) {
  for (struct specifier *cur = list->head; cur != NULL; cur = cur->next) {
    print_specifier(cur);
  }
}

void print_statement(struct statement *stmt);
void print_expression(struct expression *expr);
void print_declaration(struct declaration *decl);

void print_initialized_declarator(struct initialized_declarator *decl) {
  print("Initialized declarator");

  stack++;
  print_id(decl->declarator);

  if (decl->initializer)
    print_expression(decl->initializer);
  stack--;
}

void print_init_declarator_list(struct init_declarator_list *list) {
  if (list == NULL)
    return;

  print("Init declarator list");

  stack++;
  for (struct initialized_declarator *cur = list->head; cur != NULL;
       cur = cur->next) {
    print_initialized_declarator(cur);
  }
  stack--;
}

void print_declaration_list(struct declaration_list *list) {
  struct declaration *cur = list->head;
  struct declaration *next = NULL;

  while (cur) {
    next = cur->next;
    print_declaration(cur);
    cur = next;
  }
}

void print_variable_definition(struct declaration *decl) {
  print("Variable definition");

  stack++;
  print_specifier_list(decl->_var.specifiers);
  print_init_declarator_list(decl->_var.init_declarator_list);
  stack--;
}

void print_function_definition(struct declaration *decl) {
  print("Function");

  stack++;
  print_specifier_list(decl->_func.specifiers);
  print_id(decl->_func.name);

  if (decl->_func.parameters)
    print_declaration_list(decl->_func.parameters);

  print_statement(decl->_func.body);

  print("Parameter scope: %p", decl->_func.parameter_scope);
  stack--;
}

void print_type_definition(struct declaration *decl) {
  print("Type definition");

  stack++;
  print_specifier_list(decl->_type_def.specifiers);
  print_id(decl->_type_def.name);
  stack--;
}

void print_declaration(struct declaration *decl) {
  switch (decl->type) {
  case VARIABLE:
    print_variable_definition(decl);
    break;
  case FUNCTION:
    print_function_definition(decl);
    break;
  case TYPEDEF:
    print_type_definition(decl);
    break;
  default:
    CRITICAL("ast", "Unknown declaration type");
  }
}

void print_break_stmt(struct statement *stmt) {
  UNUSED(stmt);

  print("Break");
}

void print_statement_list(struct statement_list *list) {
  if (list == NULL)
    return;

  for (struct statement *child = list->head; child != NULL;
       child = child->next) {
    print_statement(child);
  }
}

void print_compound_stmt(struct statement *stmt) {
  print("Compound statement");

  stack++;
  print_statement_list(stmt->_compound.statements);
  print("Local scope: %p", stmt->_compound.local_scope);
  stack--;
}

void print_continue_stmt(struct statement *stmt) {
  UNUSED(stmt);
  print("Continue");
}

void print_decl_stmt(struct statement *stmt) {
  print("Declaration statement");

  stack++;
  print_declaration(stmt->_decl);
  stack--;
}

void print_expr_stmt(struct statement *stmt) {
  print("Expression");

  stack++;
  print_expression(stmt->_expr);
  stack--;
}

void print_for_stmt(struct statement *stmt) {
  print("For");

  stack++;

  if (stmt->_for.decl) {
    print_declaration(stmt->_for.decl);
  }

  if (stmt->_for.preloop_expression) {
    print_expression(stmt->_for.preloop_expression);
  }

  if (stmt->_for.condition) {
    print_expression(stmt->_for.condition);
  }

  if (stmt->_for.step_expression) {
    print_expression(stmt->_for.step_expression);
  }

  if (stmt->_for.body) {
    print_statement(stmt->_for.body);
  }

  stack--;
}

void print_goto_stmt(struct statement *stmt) {
  print("Goto");

  stack++;
  print_id(stmt->_goto);
  stack--;
}

void print_if_stmt(struct statement *stmt) {
  print("If");

  stack++;

  if (stmt->_if.condition) {
    print_expression(stmt->_if.condition);
  }

  if (stmt->_if.body) {
    print_statement(stmt->_if.body);
  }

  if (stmt->_if.else_body) {
    print_statement(stmt->_if.else_body);
  }

  stack--;
}

void print_label_stmt(struct statement *stmt) {
  print("Label");

  stack++;
  print_id(stmt->_label.name);
  stack--;
}

void print_return_stmt(struct statement *stmt) {
  print("Return");

  stack++;
  if (stmt->_return.ret_expr) {
    print_expression(stmt->_return.ret_expr);
  }
  stack--;
}

void print_switch_stmt(struct statement *stmt) {
  print("Switch");

  stack++;

  if (stmt->_switch.condition) {
    print_expression(stmt->_switch.condition);
  }

  if (stmt->_switch.body) {
    print_statement(stmt->_switch.body);
  }

  stack--;
}

void print_switch_label_stmt(struct statement *stmt) {
  print("Switch label");

  stack++;
  if (stmt->_switch_label.test) {
    print_expression(stmt->_switch_label.test);
  }
  stack--;
}

void print_while_stmt(struct statement *stmt) {
  if (stmt->_while.should_check_condition_first) {
    print("While");
  } else {
    print("Do while");
  }

  stack++;
  if (stmt->_while.condition) {
    print_expression(stmt->_while.condition);
  }

  if (stmt->_while.body) {
    print_statement(stmt->_while.body);
  }
  stack--;
}

void print_statement(struct statement *stmt) {
  switch (stmt->type) {
  case BREAK:
    print_break_stmt(stmt);
    break;
  case COMPOUND:
    print_compound_stmt(stmt);
    break;
  case CONTINUE:
    print_continue_stmt(stmt);
    break;
  case DECL:
    print_decl_stmt(stmt);
    break;
  case EXPR:
    print_expr_stmt(stmt);
    break;
  case FOR:
    print_for_stmt(stmt);
    break;
  case GOTO:
    print_goto_stmt(stmt);
    break;
  case IF:
    print_if_stmt(stmt);
    break;
  case LABEL:
    print_label_stmt(stmt);
    break;
  case RETURN:
    print_return_stmt(stmt);
    break;
  case SWITCH:
    print_switch_stmt(stmt);
    break;
  case SWITCH_LABEL:
    print_switch_label_stmt(stmt);
    break;
  case WHILE:
    print_while_stmt(stmt);
    break;
  }
}

void print_expression_list(struct expression_list *list);

void print_index_expression(struct expression *expr) {
  switch (expr->_index.type) {
  case ARRAY:
    print("Array index");
    break;
  case DOT:
    print("Dot index");
    break;
  case ARROW:
    print("Arrow index");
    break;
  default:
    print("Unknown index type");
  }

  stack++;
  if (expr->_index.object) {
    print("Object");
    stack++;
    print_expression(expr->_index.object);
    stack--;
  }

  if (expr->_index.index) {
    print("Index");
    stack++;
    print_expression(expr->_index.index);
    stack--;
  }
  stack--;
}

void print_func_call(struct expression *expr) {
  print("Function call");

  stack++;

  if (expr->_call.function_ptr) {
    print("Function pointer");
    stack++;
    print_expression(expr->_call.function_ptr);
    stack--;
  }

  if (expr->_call.parameter_list) {
    print("Parameter list");
    stack++;
    print_expression_list(expr->_call.parameter_list);
    stack--;
  }

  stack--;
}

void print_postfix_expr(struct expression *expr) {
  print("Postfix expression");

  stack++;
  if (expr->_unary.base) {
    print_expression(expr->_unary.base);
  }

  if (expr->_unary.operator) {
    print_token(expr->_unary.operator);
  }
  stack--;
}

void print_unary_expr(struct expression *expr) {
  print("Unary expression");

  stack++;
  if (expr->_unary.operator) {
    print_token(expr->_unary.operator);
  }

  if (expr->_unary.base) {
    print_expression(expr->_unary.base);
  }
  stack--;
}

void print_cast_expr(struct expression *expr) {
  print("Cast expression");

  stack++;

  if (expr->_cast.type) {
    print_specifier_list(expr->_cast.type);
  }

  if (expr->_cast.base) {
    print_expression(expr->_cast.base);
  }

  stack--;
}

void print_binary_expr(struct expression *expr) {
  print("Binary expression");

  stack++;

  if (expr->_binary.left) {
    print_expression(expr->_binary.left);
  }

  if (expr->_binary.operator) {
    print_token(expr->_binary.operator);
  }

  if (expr->_binary.right) {
    print_expression(expr->_binary.right);
  }

  stack--;
}

void print_ternary_expr(struct expression *expr) {
  print("Ternary expression");

  stack++;

  if (expr->_ternary.condition) {
    print_expression(expr->_ternary.condition);
  }

  if (expr->_ternary.true_branch) {
    print_expression(expr->_ternary.true_branch);
  }

  if (expr->_ternary.false_branch) {
    print_expression(expr->_ternary.false_branch);
  }

  stack--;
}

void print_expression(struct expression *expr) {
  if (expr == NULL) {
    print("(null)");
    return;
  }

  switch (expr->type) {
  case CONST_EXPR:
    print("Constant");
    stack++;
    print_token(expr->_constant);
    stack--;
    break;
  case ID_EXPR:
    print("Id expression");
    stack++;
    print_id(expr->_id);
    stack--;
    break;
  case INDEX:
    print_index_expression(expr);
    break;
  case FUNC_CALL:
    print_func_call(expr);
    break;
  case POSTFIX:
    print_postfix_expr(expr);
    break;
  case UNARY:
    print_unary_expr(expr);
    break;
  case CAST:
    print_cast_expr(expr);
    break;
  case BINARY:
    print_binary_expr(expr);
    break;
  case TERNARY:
    print_ternary_expr(expr);
    break;
  default:
    print("Unknown expression type");
  }

  print("Register: r%lu", expr->reg);
  print("Register count: %zu", expr->reg_count);
}

void print_expression_list(struct expression_list *list) {
  if (list == NULL) {
    print("(null)");
    return;
  }

  struct expression *cur = list->head;

  if (cur == NULL) {
    print("(empty list)");
    return;
  }

  while (cur != NULL) {
    print_expression(cur);
    cur = cur->next;
  }
}

void print_translation_unit(struct translation_unit *unit) {
  print("Translation Unit");

  stack++;
  print_declaration_list(&unit->external_declarations);
  print("Global scope: %p", unit->global_scope);
  stack--;
}
