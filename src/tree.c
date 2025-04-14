#include "tree.h"
#include "log.h"

#define NEW(ty) allocate_or_error(sizeof(ty))

struct allocation {
  void *address;
  struct allocation *next;
};

struct allocation *allocation_list;

AST root = NULL;

void *allocate_or_error(size_t size) {
  void *result = malloc(size);
  struct allocation *allocation = malloc(sizeof(struct allocation));

  if (result == NULL || allocation == NULL) {
    CRITICAL("ast", "Out of memory!");
  }

  allocation->address = result;
  allocation->next = allocation_list;
  allocation_list = allocation;

  return result;
}

// Creating functions

struct id *create_id(Token *name) {
  struct id *result = NEW(struct id);

  result->name = name;

  return result;
}

struct declaration *create_declaration(struct specifier_list *specifiers,
                                       struct id *identifier) {
  struct declaration *decl = NEW(struct declaration);

  decl->is_type_definition = false;
  decl->specifiers = specifiers;
  decl->name = identifier;
  decl->init_declarator_list = NULL;
  decl->body = NULL;
  decl->next = NULL;

  return decl;
}

struct declaration *
create_init_declaration(struct specifier_list *specifiers,
                        struct init_declarator_list *init_declarator_list) {
  struct declaration *decl = NEW(struct declaration);

  decl->is_type_definition = false;
  decl->specifiers = specifiers;
  decl->name = NULL;
  decl->init_declarator_list = init_declarator_list;
  decl->body = NULL;
  decl->next = NULL;

  return decl;
}

struct declaration *create_function(struct specifier_list *specifiers,
                                    struct id *identifier,
                                    struct statement *body) {
  struct declaration *decl = NEW(struct declaration);

  decl->is_type_definition = false;
  decl->specifiers = specifiers;
  decl->name = identifier;
  decl->init_declarator_list = NULL;
  decl->body = body;
  decl->next = NULL;

  return decl;
}

struct declaration *create_type_definition(struct specifier_list *specifiers,
                                           struct id *identifier) {
  struct declaration *decl = NEW(struct declaration);

  decl->is_type_definition = true;
  decl->specifiers = specifiers;
  decl->name = identifier;
  decl->init_declarator_list = NULL;
  decl->body = NULL;
  decl->next = NULL;

  return decl;
}

void append_declaration(struct declaration_list *list,
                        struct declaration *decl) {
  if (list->head == NULL) {
    list->head = decl;
  }

  if (list->tail) {
    list->tail->next = decl;
  }

  list->tail = decl;
}

struct translation_unit *create_translation_unit(struct declaration *first) {
  struct translation_unit *unit = NEW(struct translation_unit);

  unit->external_declarations.head = first;
  unit->external_declarations.tail = first;

  root = unit;

  return unit;
}

struct translation_unit *
append_external_declaration(struct translation_unit *unit,
                            struct declaration *new_elem) {
  append_declaration(&unit->external_declarations, new_elem);
  return unit;
}

struct specifier *create_token_specifier(Token *token) {
  struct specifier *spec = NEW(struct specifier);

  spec->type = TOKEN;
  spec->_token = token;
  spec->next = NULL;

  return spec;
}

struct specifier *create_id_specifier(struct id *id) {
  struct specifier *spec = NEW(struct specifier);

  spec->type = ID_SPEC;
  spec->_id = id;
  spec->next = NULL;

  return spec;
}

struct specifier_list *create_specifier_list(struct specifier *tail) {
  struct specifier_list *list = NEW(struct specifier_list);

  list->head = tail;

  return list;
}

struct specifier_list *prepend_specifier(struct specifier *prefix,
                                         struct specifier_list *list) {
  prefix->next = list->head;
  list->head = prefix;
  return list;
}

struct statement *create_break_stmt() {
  struct statement *stmt = NEW(struct statement);

  stmt->type = BREAK;
  stmt->next = NULL;

  return stmt;
}

struct statement *create_compound_stmt(struct statement_list *list) {
  struct statement *stmt = NEW(struct statement);

  stmt->type = COMPOUND;

  if (list) {
    stmt->_compound.head = list->head;
    stmt->_compound.tail = list->tail;
    // Garbage collector will clean up the leaked list.
  } else {
    stmt->_compound.head = NULL;
    stmt->_compound.tail = NULL;
  }

  stmt->next = NULL;

  return stmt;
}

struct statement *create_continue_stmt() {
  struct statement *stmt = NEW(struct statement);

  stmt->type = CONTINUE;
  stmt->next = NULL;

  return stmt;
}

struct statement *create_decl_stmt(struct declaration *decl) {
  struct statement *stmt = NEW(struct statement);

  stmt->type = DECL;
  stmt->_decl = decl;
  stmt->next = NULL;

  return stmt;
}

struct statement *create_expr_stmt(struct expression *expr) {
  struct statement *stmt = NEW(struct statement);

  stmt->type = EXPR;
  stmt->_expr = expr;
  stmt->next = NULL;

  return stmt;
}

struct statement *create_for_stmt(struct declaration *decl,
                                  struct statement *body) {
  struct statement *stmt = NEW(struct statement);

  stmt->type = FOR;

  stmt->_for.is_initializer_decl = decl != NULL;
  stmt->_for.decl = decl;
  stmt->_for.body = body;

  stmt->next = NULL;

  return stmt;
}

struct statement *create_goto_stmt(struct id *label) {
  struct statement *stmt = NEW(struct statement);

  stmt->type = GOTO;
  stmt->_goto = label;
  stmt->next = NULL;

  return stmt;
}

struct statement *create_if_stmt(struct statement *body,
                                 struct statement *else_body) {
  struct statement *stmt = NEW(struct statement);

  stmt->type = IF;
  stmt->_if.body = body;
  stmt->_if.else_body = else_body;
  stmt->next = NULL;

  return stmt;
}

struct statement *create_label_stmt(struct id *name) {
  struct statement *stmt = NEW(struct statement);

  stmt->type = LABEL;
  stmt->_label.name = name;
  stmt->next = NULL;

  return stmt;
}

struct statement *create_return_stmt(struct expression *expr) {
  struct statement *stmt = NEW(struct statement);

  stmt->type = RETURN;
  stmt->_return.ret_expr = expr;
  stmt->next = NULL;

  return stmt;
}

struct statement *create_switch_stmt(struct statement *body) {
  struct statement *stmt = NEW(struct statement);

  stmt->type = SWITCH;
  stmt->_switch.body = body;
  stmt->next = NULL;

  return stmt;
}

struct statement *create_case_stmt() {
  struct statement *stmt = NEW(struct statement);

  stmt->type = SWITCH_LABEL;
  stmt->next = NULL;

  return stmt;
}

struct statement *create_default_stmt() {
  struct statement *stmt = NEW(struct statement);

  stmt->type = SWITCH_LABEL;
  stmt->next = NULL;

  return stmt;
}

struct statement *create_do_while_stmt(struct statement *body) {
  struct statement *stmt = NEW(struct statement);

  stmt->type = WHILE;
  stmt->_while.should_check_condition_first = false;
  stmt->_while.body = body;
  stmt->next = NULL;

  return stmt;
}

struct statement *create_while_stmt(struct statement *body) {
  struct statement *stmt = NEW(struct statement);

  stmt->type = WHILE;
  stmt->_while.should_check_condition_first = true;
  stmt->_while.body = body;
  stmt->next = NULL;

  return stmt;
}

struct statement_list *create_stmt_list(struct statement *first) {
  struct statement_list *list = NEW(struct statement_list);

  list->head = first;
  list->tail = first;

  return list;
}

struct statement_list *prepend_stmt(struct statement *new_stmt,
                                    struct statement_list *list) {
  new_stmt->next = list->head;
  list->head = new_stmt;

  if (list->tail == NULL)
    list->tail = new_stmt;

  return list;
}

struct statement_list *append_stmt(struct statement_list *list,
                                   struct statement *new_stmt) {
  if (list->head == NULL) {
    list->head = new_stmt;
  }

  if (list->tail) {
    list->tail->next = new_stmt;
  }

  list->tail = new_stmt;

  return list;
}

struct expression *create_id_expression(struct id *id) {
  struct expression *expr = NEW(struct expression);

  expr->type = ID_EXPR;
  expr->_id = id;
  expr->next = NULL;

  return expr;
}

struct expression *create_const_expression(Token *constant) {
  struct expression *expr = NEW(struct expression);

  expr->type = CONST_EXPR;
  expr->_constant = constant;
  expr->next = NULL;

  return expr;
}

struct expression *create_dot_index_expression(struct expression *obj,
                                               struct expression *index) {
  struct expression *expr = NEW(struct expression);

  expr->type = INDEX;
  expr->_index.type = DOT;
  expr->_index.object = obj;
  expr->_index.index = index;
  expr->next = NULL;

  return expr;
}

struct expression *create_arrow_index_expression(struct expression *obj,
                                                 struct expression *index) {
  struct expression *expr = NEW(struct expression);

  expr->type = INDEX;
  expr->_index.type = ARROW;
  expr->_index.object = obj;
  expr->_index.index = index;
  expr->next = NULL;

  return expr;
}

struct expression *create_array_index_expression(struct expression *obj,
                                                 struct expression *index) {
  struct expression *expr = NEW(struct expression);

  expr->type = INDEX;
  expr->_index.type = ARRAY;
  expr->_index.object = obj;
  expr->_index.index = index;
  expr->next = NULL;

  return expr;
}

struct expression *
create_call_expression(struct expression *function_ptr,
                       struct expression_list *parameter_list) {
  struct expression *expr = NEW(struct expression);

  expr->type = FUNC_CALL;
  expr->_call.function_ptr = function_ptr;
  expr->_call.parameter_list = parameter_list;
  expr->next = NULL;

  return expr;
}

struct expression *create_postfix_expression(struct expression *base,
                                             Token *operator) {
  struct expression *expr = NEW(struct expression);

  expr->type = POSTFIX;
  expr->_unary.base = base;
  expr->_unary.operator= operator;
  expr->next = NULL;

  return expr;
}

struct expression *create_unary_expression(Token *operator,
                                           struct expression * base) {
  struct expression *expr = NEW(struct expression);

  expr->type = UNARY;
  expr->_unary.base = base;
  expr->_unary.operator= operator;
  expr->next = NULL;

  return expr;
}

struct expression *create_cast_expression(struct specifier_list *type,
                                          struct expression *base) {
  struct expression *expr = NEW(struct expression);

  expr->type = CAST;
  expr->_cast.type = type;
  expr->_cast.base = base;
  expr->next = NULL;

  return expr;
}

struct expression *create_binary_expression(struct expression *left,
                                            Token *operator,
                                            struct expression * right) {
  struct expression *expr = NEW(struct expression);

  expr->type = BINARY;
  expr->_binary.operator= operator;
  expr->_binary.left = left;
  expr->_binary.right = right;
  expr->next = NULL;

  return expr;
}

struct expression *create_ternary_expression(struct expression *condition,
                                             struct expression *true_branch,
                                             struct expression *false_branch) {
  struct expression *expr = NEW(struct expression);

  expr->type = TERNARY;
  expr->_ternary.condition = condition;
  expr->_ternary.true_branch = true_branch;
  expr->_ternary.false_branch = false_branch;
  expr->next = NULL;

  return expr;
}

struct expression_list *create_expr_list(struct expression *first_elem) {
  struct expression_list *list = NEW(struct expression_list);

  list->head = first_elem;
  list->tail = first_elem;

  return list;
}

struct expression_list *append_expr(struct expression_list *list,
                                    struct expression *expr) {
  if (list->head == NULL)
    list->head = expr;

  if (list->tail)
    list->tail->next = expr;

  list->tail = expr;

  return list;
}

struct initialized_declarator *
create_initialized_declarator(struct id *declarator,
                              struct expression *initializer) {
  struct initialized_declarator *decl = NEW(struct initialized_declarator);

  decl->declarator = declarator;
  decl->initializer = initializer;
  decl->next = NULL;

  return decl;
}

struct init_declarator_list *
create_init_declarator_list(struct initialized_declarator *first_elem) {
  struct init_declarator_list *list = NEW(struct init_declarator_list);

  list->head = first_elem;
  list->tail = first_elem;

  return list;
}

struct init_declarator_list *
append_initialized_declarator(struct init_declarator_list *list,
                              struct initialized_declarator *new_elem) {
  if (list->head == NULL)
    list->head = new_elem;

  if (list->tail)
    list->tail->next = new_elem;

  list->tail = new_elem;
  return list;
}

// Destroying functions

void destroy_id(struct id *id) {
  if (id)
    free(id);
}

void destroy_specifier(struct specifier *specifier) {
  switch (specifier->type) {
  case TOKEN: /* Do nothing */
    break;
  case ID_SPEC:
    destroy_id(specifier->_id);
    break;
  }

  free(specifier);
}

void destroy_specifiers(struct specifier_list *list) {
  struct specifier *cur = list->head;
  struct specifier *next = NULL;

  while (cur) {
    next = cur->next;
    destroy_specifier(cur);
    cur = next;
  }

  free(list);
}

void destroy_statement(struct statement *stmt);
void destroy_expression(struct expression *expr);

void destory_initialized_declarator(struct initialized_declarator *decl) {
  destroy_id(decl->declarator);

  if (decl->initializer)
    destroy_expression(decl->initializer);

  free(decl);
}

void destory_init_declarator_list(struct init_declarator_list *list) {
  struct initialized_declarator *cur = list->head;
  struct initialized_declarator *next = NULL;

  while (cur) {
    next = cur->next;
    destory_initialized_declarator(cur);
    cur = next;
  }

  free(list);
}

void destroy_declaration(struct declaration *decl) {
  destroy_specifiers(decl->specifiers);
  destroy_id(decl->name);

  if (decl->init_declarator_list)
    destory_init_declarator_list(decl->init_declarator_list);

  if (decl->body)
    destroy_statement(decl->body);

  free(decl);
}

void destroy_break_stmt(struct statement *stmt) { free(stmt); }

void destroy_compound_stmt(struct statement *stmt) {
  struct statement *cur = stmt->_compound.head;
  struct statement *next = NULL;

  while (cur != NULL) {
    next = cur->next;
    destroy_statement(cur);
    cur = next;
  }

  free(stmt);
}

void destroy_continue_stmt(struct statement *stmt) { free(stmt); }

void destroy_decl_stmt(struct statement *stmt) {
  destroy_declaration(stmt->_decl);
  free(stmt);
}

void destroy_expr_stmt(struct statement *stmt) {
  if (stmt->_expr) {
    destroy_expression(stmt->_expr);
  }

  free(stmt);
}

void destroy_for_stmt(struct statement *stmt) {
  if (stmt->_for.decl) {
    destroy_declaration(stmt->_for.decl);
  }

  if (stmt->_for.body) {
    destroy_statement(stmt->_for.body);
  }

  free(stmt);
}

void destroy_goto_stmt(struct statement *stmt) {
  destroy_id(stmt->_goto);
  free(stmt);
}

void destroy_if_stmt(struct statement *stmt) {
  if (stmt->_if.body) {
    destroy_statement(stmt->_if.body);
  }

  if (stmt->_if.else_body) {
    destroy_statement(stmt->_if.else_body);
  }

  free(stmt);
}

void destroy_label_stmt(struct statement *stmt) {
  destroy_id(stmt->_label.name);
  free(stmt);
}

void destroy_return_stmt(struct statement *stmt) {
  if (stmt->_return.ret_expr) {
    destroy_expression(stmt->_return.ret_expr);
  }

  free(stmt);
}

void destroy_switch_stmt(struct statement *stmt) {
  if (stmt->_switch.body) {
    destroy_statement(stmt->_switch.body);
  }

  free(stmt);
}

void destroy_switch_label_stmt(struct statement *stmt) { free(stmt); }

void destroy_while_stmt(struct statement *stmt) {
  if (stmt->_while.body) {
    destroy_statement(stmt->_while.body);
  }

  free(stmt);
}

void destroy_statement(struct statement *stmt) {
  switch (stmt->type) {
  case BREAK:
    destroy_break_stmt(stmt);
    break;
  case COMPOUND:
    destroy_compound_stmt(stmt);
    break;
  case CONTINUE:
    destroy_continue_stmt(stmt);
    break;
  case DECL:
    destroy_decl_stmt(stmt);
    break;
  case EXPR:
    destroy_expr_stmt(stmt);
    break;
  case FOR:
    destroy_for_stmt(stmt);
    break;
  case GOTO:
    destroy_goto_stmt(stmt);
    break;
  case IF:
    destroy_if_stmt(stmt);
    break;
  case LABEL:
    destroy_label_stmt(stmt);
    break;
  case RETURN:
    destroy_return_stmt(stmt);
    break;
  case SWITCH:
    destroy_switch_stmt(stmt);
    break;
  case SWITCH_LABEL:
    destroy_switch_label_stmt(stmt);
    break;
  case WHILE:
    destroy_while_stmt(stmt);
    break;
  }
}

void destroy_expr_list(struct expression_list *list) {
  struct expression *cur = list->head;
  struct expression *next = NULL;

  while (cur) {
    next = cur->next;
    destroy_expression(cur);
    cur = next;
  }

  free(list);
}

void destroy_id_expression(struct expression *expr) {
  destroy_id(expr->_id);
  free(expr);
}

void destroy_const_expression(struct expression *expr) { free(expr); }

void destroy_index_expression(struct expression *expr) {
  destroy_expression(expr->_index.object);
  destroy_expression(expr->_index.index);
  free(expr);
}

void destroy_call_expression(struct expression *expr) {
  destroy_expression(expr->_call.function_ptr);
  destroy_expr_list(expr->_call.parameter_list);
  free(expr);
}

void destroy_unary_expression(struct expression *expr) {
  destroy_expression(expr->_unary.base);
  free(expr);
}

void destroy_cast_expression(struct expression *expr) {
  destroy_specifiers(expr->_cast.type);
  destroy_expression(expr->_cast.base);
  free(expr);
}

void destroy_binary_expression(struct expression *expr) {
  destroy_expression(expr->_binary.left);
  destroy_expression(expr->_binary.right);
  free(expr);
}

void destroy_ternary_expression(struct expression *expr) {
  destroy_expression(expr->_ternary.condition);
  destroy_expression(expr->_ternary.true_branch);
  destroy_expression(expr->_ternary.false_branch);
  free(expr);
}

void destroy_expression(struct expression *expr) {
  switch (expr->type) {
  case ID_EXPR:
    destroy_id_expression(expr);
    break;
  case CONST_EXPR:
    destroy_const_expression(expr);
    break;
  case INDEX:
    destroy_index_expression(expr);
    break;
  case FUNC_CALL:
    destroy_call_expression(expr);
    break;
  case POSTFIX:
    destroy_unary_expression(expr);
    break;
  case UNARY:
    destroy_unary_expression(expr);
    break;
  case CAST:
    destroy_cast_expression(expr);
    break;
  case BINARY:
    destroy_binary_expression(expr);
    break;
  case TERNARY:
    destroy_ternary_expression(expr);
    break;
  }
}

void destroy_translation_unit(struct translation_unit *unit) {
  struct declaration *cur = unit->external_declarations.head;
  struct declaration *next = NULL;

  while (cur) {
    next = cur->next;
    destroy_declaration(cur);
    cur = next;
  }

  free(unit);
}

// Sense allocations
void delete_allocation_entry(void *address) {
  struct allocation *prev = NULL;
  for (struct allocation *cur = allocation_list; cur != NULL; cur = cur->next) {
    if (cur->address != address) {
      prev = cur;
      continue;
    }

    if (prev) {
      prev->next = cur->next;
    } else {
      allocation_list = cur->next;
    }

    free(cur);
    break;
  }
}

void sense_id(struct id *id) {
  if (id)
    delete_allocation_entry(id);
}

void sense_specifier(struct specifier *specifier) {
  switch (specifier->type) {
  case TOKEN: /* Do nothing */
    break;
  case ID_SPEC:
    sense_id(specifier->_id);
    break;
  }

  delete_allocation_entry(specifier);
}

void sense_specifiers(struct specifier_list *list) {
  struct specifier *cur = list->head;
  struct specifier *next = NULL;

  while (cur) {
    next = cur->next;
    sense_specifier(cur);
    cur = next;
  }

  delete_allocation_entry(list);
}

void sense_statement(struct statement *stmt);
void sense_expression(struct expression *expr);

void sense_initialized_declarator(struct initialized_declarator *decl) {
  sense_id(decl->declarator);

  if (decl->initializer)
    sense_expression(decl->initializer);

  delete_allocation_entry(decl);
}

void sense_init_declarator_list(struct init_declarator_list *list) {
  struct initialized_declarator *cur = list->head;
  struct initialized_declarator *next = NULL;

  while (cur) {
    next = cur->next;
    sense_initialized_declarator(cur);
    cur = next;
  }

  delete_allocation_entry(list);
}

void sense_declaration(struct declaration *decl) {
  sense_specifiers(decl->specifiers);
  sense_id(decl->name);

  if (decl->init_declarator_list)
    sense_init_declarator_list(decl->init_declarator_list);

  if (decl->body)
    sense_statement(decl->body);

  delete_allocation_entry(decl);
}

void sense_compound_stmt(struct statement *stmt) {
  for (struct statement *child = stmt->_compound.head; child != NULL;
       child = child->next) {
    sense_statement(child);
  }
}

void sense_decl_stmt(struct statement *stmt) { sense_declaration(stmt->_decl); }

void sense_for_stmt(struct statement *stmt) {
  if (stmt->_for.decl) {
    sense_declaration(stmt->_for.decl);
  }

  if (stmt->_for.body) {
    sense_statement(stmt->_for.body);
  }
}

void sense_goto_stmt(struct statement *stmt) { sense_id(stmt->_goto); }

void sense_if_stmt(struct statement *stmt) {
  if (stmt->_if.body) {
    sense_statement(stmt->_if.body);
  }

  if (stmt->_if.else_body) {
    sense_statement(stmt->_if.else_body);
  }
}

void sense_label_stmt(struct statement *stmt) { sense_id(stmt->_label.name); }

void sense_switch_stmt(struct statement *stmt) {
  if (stmt->_switch.body) {
    sense_statement(stmt->_switch.body);
  }
}

void sense_while_stmt(struct statement *stmt) {
  if (stmt->_while.body) {
    sense_statement(stmt->_while.body);
  }
}

void sense_statement(struct statement *stmt) {
  switch (stmt->type) {
  case BREAK:
    break;
  case COMPOUND:
    sense_compound_stmt(stmt);
    break;
  case CONTINUE:
    break;
  case DECL:
    sense_decl_stmt(stmt);
    break;
  case EXPR:
    if (stmt->_expr) {
      sense_expression(stmt->_expr);
    }

    break;
  case FOR:
    sense_for_stmt(stmt);
    break;
  case GOTO:
    sense_goto_stmt(stmt);
    break;
  case IF:
    sense_if_stmt(stmt);
    break;
  case LABEL:
    sense_label_stmt(stmt);
    break;
  case RETURN:
    if (stmt->_return.ret_expr) {
      sense_expression(stmt->_return.ret_expr);
    }

    break;
  case SWITCH:
    sense_switch_stmt(stmt);
    break;
  case SWITCH_LABEL:
    break;
  case WHILE:
    sense_while_stmt(stmt);
    break;
  }

  delete_allocation_entry(stmt);
}

void sense_expr_list(struct expression_list *list) {
  struct expression *cur = list->head;
  struct expression *next = NULL;

  while (cur) {
    next = cur->next;
    sense_expression(cur);
    cur = next;
  }

  delete_allocation_entry(list);
}

void sense_id_expression(struct expression *expr) {
  sense_id(expr->_id);
  delete_allocation_entry(expr);
}

void sense_const_expression(struct expression *expr) {
  delete_allocation_entry(expr);
}

void sense_index_expression(struct expression *expr) {
  sense_expression(expr->_index.object);
  sense_expression(expr->_index.index);
  delete_allocation_entry(expr);
}

void sense_call_expression(struct expression *expr) {
  sense_expression(expr->_call.function_ptr);
  sense_expr_list(expr->_call.parameter_list);
  delete_allocation_entry(expr);
}

void sense_unary_expression(struct expression *expr) {
  sense_expression(expr->_unary.base);
  delete_allocation_entry(expr);
}

void sense_cast_expression(struct expression *expr) {
  sense_specifiers(expr->_cast.type);
  sense_expression(expr->_cast.base);
  delete_allocation_entry(expr);
}

void sense_binary_expression(struct expression *expr) {
  sense_expression(expr->_binary.left);
  sense_expression(expr->_binary.right);
  delete_allocation_entry(expr);
}

void sense_ternary_expression(struct expression *expr) {
  sense_expression(expr->_ternary.condition);
  sense_expression(expr->_ternary.true_branch);
  sense_expression(expr->_ternary.false_branch);
  delete_allocation_entry(expr);
}

void sense_expression(struct expression *expr) {
  switch (expr->type) {
  case ID_EXPR:
    sense_id_expression(expr);
    break;
  case CONST_EXPR:
    sense_const_expression(expr);
    break;
  case INDEX:
    sense_index_expression(expr);
    break;
  case FUNC_CALL:
    sense_call_expression(expr);
    break;
  case POSTFIX:
    sense_unary_expression(expr);
    break;
  case UNARY:
    sense_unary_expression(expr);
    break;
  case CAST:
    sense_cast_expression(expr);
    break;
  case BINARY:
    sense_binary_expression(expr);
    break;
  case TERNARY:
    sense_ternary_expression(expr);
    break;
  }
}

void sense_translation_unit(struct translation_unit *unit) {
  struct declaration *cur = unit->external_declarations.head;
  struct declaration *next = NULL;

  while (cur) {
    next = cur->next;
    sense_declaration(cur);
    cur = next;
  }

  delete_allocation_entry(unit);
}

AST get_tree() { return root; }

void free_unused_parse_branches() {
  if (!root)
    return;

  sense_translation_unit(root);

  size_t unused_allocations_count = 0;
  struct allocation *cur = allocation_list;
  allocation_list = NULL;

  struct allocation *next = NULL;
  while (cur != NULL) {
    next = cur->next;
    free(cur->address);
    free(cur);
    cur = next;
    unused_allocations_count++;
  }

#ifndef NDEBUG
  DEBUG("Freed %zu unused blocks", unused_allocations_count);
#endif
}

void destroy_ast() {
  if (root)
    destroy_translation_unit(root);
}
