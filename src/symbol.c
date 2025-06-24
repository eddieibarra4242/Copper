#include "symbol.h"
#include "common.h"
#include "log.h"
#include "tree.h"

#include <stdlib.h>
#include <string.h>

struct scope *current_scope = NULL;

struct scope *create_scope(void) {
  struct scope *new_scope = malloc(sizeof(struct scope));
  new_scope->parent = current_scope;
  new_scope->symbols = NULL;

  current_scope = new_scope;

  return new_scope;
}

void destroy_scope(struct scope *scope) {
  if (scope == NULL) {
    return;
  }

  struct symbol *symbol = scope->symbols;
  struct symbol *next_symbol;

  while (symbol != NULL) {
    next_symbol = symbol->next;
    free(symbol);
    symbol = next_symbol;
  }

  free(scope);
}

void create_symbol(struct id *id) {
  if (current_scope == NULL) {
    CRITICAL("assign", "Cannot create symbol without a scope");
  }

  struct symbol *new_symbol = malloc(sizeof(struct symbol));
  new_symbol->name = id->name->data;
  new_symbol->next = current_scope->symbols;
  current_scope->symbols = new_symbol;
}

struct symbol *find_symbol(Token *name) {
  struct scope *scope = current_scope;

  while (scope != NULL) {
    struct symbol *symbol = scope->symbols;

    while (symbol != NULL) {
      if (strcmp(symbol->name, name->data) == 0) {
         // Found the symbol
        return symbol;
      }

      symbol = symbol->next;
    }

    scope = scope->parent;
  }

  return NULL;
}

void symbol_translation_unit(struct translation_unit *unit);

// Entry point
void link_symbols(void) {
  AST tree = get_tree();
  symbol_translation_unit(tree);
}

void symbol_id(struct id *id) {
  struct symbol *symbol = find_symbol(id->name);

  if (symbol == NULL) {
    // todo: accumulate errors
    ERRORV("assign", "Cannot find declaration for symbol '%s' (seen on line %zu:%zu)", id->name->data,
           id->name->span.start.line_number, id->name->span.start.column);
  }

  id->symbol = symbol;
}

void symbol_token_specifier(struct specifier *specifier) {
  UNUSED(specifier);
  // specifier->_token
}

void symbol_id_specifier(struct specifier *specifier) { symbol_id(specifier->_id); }

void symbol_specifier(struct specifier *specifier) {
  switch (specifier->type) {
  case TOKEN:
    symbol_token_specifier(specifier);
    break;
  case ID_SPEC:
    symbol_id_specifier(specifier);
    break;
  default:
    CRITICAL("assign", "Unknown specifier type");
  }
}

void symbol_specifier_list(struct specifier_list *list) {
  for (struct specifier *cur = list->head; cur != NULL; cur = cur->next) {
    symbol_specifier(cur);
  }
}

void symbol_statement(struct statement *stmt);
void symbol_expression(struct expression *expr);

void symbol_initialized_declarator(struct initialized_declarator *decl) {
  create_symbol(decl->declarator);
  symbol_id(decl->declarator);

  if (decl->initializer)
    symbol_expression(decl->initializer);
}

void symbol_init_declarator_list(struct init_declarator_list *list) {
  for (struct initialized_declarator *cur = list->head; cur != NULL;
       cur = cur->next) {
    symbol_initialized_declarator(cur);
  }
}

void symbol_declaration(struct declaration *decl) {
  if (decl->specifiers)
    symbol_specifier_list(decl->specifiers);

  if (decl->name) {
    create_symbol(decl->name);
    symbol_id(decl->name);
  }

  if (decl->body) {
    decl->parameter_scope = create_scope();

    // current scope is already set to the new scope
    symbol_statement(decl->body);

    // restore the previous scope after the body has been processed
    current_scope = current_scope->parent;
  }

  if (decl->init_declarator_list) {
    symbol_init_declarator_list(decl->init_declarator_list);
  }
}

void symbol_declaration_list(struct declaration_list *list) {
  for (struct declaration *cur = list->head; cur != NULL; cur = cur->next) {
    symbol_declaration(cur);
  }
}

void symbol_break_stmt(struct statement *stmt) { UNUSED(stmt); }

void symbol_statement_list(struct statement_list *list) {
  if (list == NULL)
    return;

  for (struct statement *child = list->head; child != NULL;
       child = child->next) {
    symbol_statement(child);
  }
}

void symbol_compound_stmt(struct statement *stmt) {
  stmt->_compound.local_scope = create_scope();

  // current scope is already set to the new scope
  symbol_statement_list(stmt->_compound.statements);

  // restore the previous scope after the body has been processed
  current_scope = current_scope->parent;
}

void symbol_continue_stmt(struct statement *stmt) { UNUSED(stmt); }

void symbol_decl_stmt(struct statement *stmt) {
  symbol_declaration(stmt->_decl);
}

void symbol_expr_stmt(struct statement *stmt) {
  symbol_expression(stmt->_expr);
}

void symbol_for_stmt(struct statement *stmt) {
  if (stmt->_for.decl) {
    symbol_declaration(stmt->_for.decl);
  }

  if (stmt->_for.preloop_expression) {
    symbol_expression(stmt->_for.preloop_expression);
  }

  if (stmt->_for.condition) {
    symbol_expression(stmt->_for.condition);
  }

  if (stmt->_for.step_expression) {
    symbol_expression(stmt->_for.step_expression);
  }

  if (stmt->_for.body) {
    symbol_statement(stmt->_for.body);
  }
}

void symbol_goto_stmt(struct statement *stmt) { symbol_id(stmt->_goto); }

void symbol_if_stmt(struct statement *stmt) {
  if (stmt->_if.condition) {
    symbol_expression(stmt->_if.condition);
  }

  if (stmt->_if.body) {
    symbol_statement(stmt->_if.body);
  }

  if (stmt->_if.else_body) {
    symbol_statement(stmt->_if.else_body);
  }
}

void symbol_label_stmt(struct statement *stmt) { symbol_id(stmt->_label.name); }

void symbol_return_stmt(struct statement *stmt) {
  if (stmt->_return.ret_expr) {
    symbol_expression(stmt->_return.ret_expr);
  }
}

void symbol_switch_stmt(struct statement *stmt) {
  if (stmt->_switch.condition) {
    symbol_expression(stmt->_switch.condition);
  }

  if (stmt->_switch.body) {
    symbol_statement(stmt->_switch.body);
  }
}

void symbol_switch_label_stmt(struct statement *stmt) {
  if (stmt->_switch_label.test) {
    symbol_expression(stmt->_switch_label.test);
  }
}

void symbol_while_stmt(struct statement *stmt) {
  // stmt->_while.should_check_condition_first

  if (stmt->_while.condition) {
    symbol_expression(stmt->_while.condition);
  }

  if (stmt->_while.body) {
    symbol_statement(stmt->_while.body);
  }
}

void symbol_statement(struct statement *stmt) {
  switch (stmt->type) {
  case BREAK:
    symbol_break_stmt(stmt);
    break;
  case COMPOUND:
    symbol_compound_stmt(stmt);
    break;
  case CONTINUE:
    symbol_continue_stmt(stmt);
    break;
  case DECL:
    symbol_decl_stmt(stmt);
    break;
  case EXPR:
    symbol_expr_stmt(stmt);
    break;
  case FOR:
    symbol_for_stmt(stmt);
    break;
  case GOTO:
    symbol_goto_stmt(stmt);
    break;
  case IF:
    symbol_if_stmt(stmt);
    break;
  case LABEL:
    symbol_label_stmt(stmt);
    break;
  case RETURN:
    symbol_return_stmt(stmt);
    break;
  case SWITCH:
    symbol_switch_stmt(stmt);
    break;
  case SWITCH_LABEL:
    symbol_switch_label_stmt(stmt);
    break;
  case WHILE:
    symbol_while_stmt(stmt);
    break;
  default:
    CRITICAL("assign", "Unknown statement type");
  }
}

void symbol_expression_list(struct expression_list *list);

void symbol_constant_expr(struct expression *expr) {
  UNUSED(expr);
  // expr->_constant
}

void symbol_id_expression(struct expression *expr) {
  symbol_id(expr->_id);
}

void symbol_index_expression(struct expression *expr) {
  if (expr->_index.object) {
    symbol_expression(expr->_index.object);
  }

  if (expr->_index.index) {
    symbol_expression(expr->_index.index);
  }
}

void symbol_func_call(struct expression *expr) {
  if (expr->_call.function_ptr) {
    symbol_expression(expr->_call.function_ptr);
  }

  if (expr->_call.parameter_list) {
    symbol_expression_list(expr->_call.parameter_list);
  }
}

void symbol_postfix_expr(struct expression *expr) {
  if (expr->_unary.base) {
    symbol_expression(expr->_unary.base);
  }

  // expr->_unary.operator
}

void symbol_unary_expr(struct expression *expr) {
  // expr->_unary.operator

  if (expr->_unary.base) {
    symbol_expression(expr->_unary.base);
  }
}

void symbol_cast_expr(struct expression *expr) {
  if (expr->_cast.type) {
    symbol_specifier_list(expr->_cast.type);
  }

  if (expr->_cast.base) {
    symbol_expression(expr->_cast.base);
  }
}

void symbol_binary_expr(struct expression *expr) {
  if (expr->_binary.left) {
    symbol_expression(expr->_binary.left);
  }

  // expr->_binary.operator

  if (expr->_binary.right) {
    symbol_expression(expr->_binary.right);
  }
}

void symbol_ternay_expr(struct expression *expr) {
  if (expr->_ternary.condition) {
    symbol_expression(expr->_ternary.condition);
  }

  if (expr->_ternary.true_branch) {
    symbol_expression(expr->_ternary.true_branch);
  }

  if (expr->_ternary.false_branch) {
    symbol_expression(expr->_ternary.false_branch);
  }
}

void symbol_expression(struct expression *expr) {
  if (expr == NULL) {
    return;
  }

  switch (expr->type) {
  case CONST_EXPR:
    symbol_constant_expr(expr);
    break;
  case ID_EXPR:
    symbol_id_expression(expr);
    break;
  case INDEX:
    symbol_index_expression(expr);
    break;
  case FUNC_CALL:
    symbol_func_call(expr);
    break;
  case POSTFIX:
    symbol_postfix_expr(expr);
    break;
  case UNARY:
    symbol_unary_expr(expr);
    break;
  case CAST:
    symbol_cast_expr(expr);
    break;
  case BINARY:
    symbol_binary_expr(expr);
    break;
  case TERNARY:
    symbol_ternay_expr(expr);
    break;
  default:
    CRITICAL("assign", "Unknown expression type");
  }
}

void symbol_expression_list(struct expression_list *list) {
  if (list == NULL) {
    return;
  }

  struct expression *cur = list->head;

  if (cur == NULL) {
    return;
  }

  while (cur != NULL) {
    symbol_expression(cur);
    cur = cur->next;
  }
}

void symbol_translation_unit(struct translation_unit *unit) {
  unit->global_scope = create_scope();

  // current scope is already set to the new scope
  symbol_declaration_list(&unit->external_declarations);

  // No need to restore the previous scope
}
