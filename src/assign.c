#include "assign.h"
#include "common.h"
#include "log.h"
#include "tree.h"
#include "utils.h"

struct sized_array {
  uint64_t *array;
  size_t size;
};

uint64_t next_reg = 0;

void translation_unit(struct translation_unit *unit);

uint64_t get_binary_reg_count(uint64_t left_reg_count,
                              uint64_t right_reg_count) {
  if (left_reg_count == right_reg_count) {
    return left_reg_count + 1;
  }

  return max(left_reg_count, right_reg_count);
}

uint64_t get_n_ary_reg_count(uint64_t *reg_count_array, size_t num_elements) {
  heap_sort(reg_count_array, num_elements);

  // calculate extra registers needed
  uint64_t extra_regs = 0;
  for (size_t i = 0; i < num_elements; i++) {
    uint64_t position = i + 1;
    long extra = reg_count_array[i] - position;

    if (extra > (long)extra_regs) {
      extra_regs = extra;
    }
  }

  return num_elements + extra_regs;
}

// Entry point
void assign_registers() {
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

void statement_list(struct statement_list *list) {
  if (list == NULL)
    return;

  for (struct statement *child = list->head; child != NULL;
       child = child->next) {
    statement(child);
  }
}

void compound_stmt(struct statement *stmt) {
  statement_list(stmt->_compound.statements);
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

void translation_unit(struct translation_unit *unit) {
  declaration_list(&unit->external_declarations);
}

void assign_expression(struct expression *expr);
void count_expression(struct expression *expr);
struct sized_array count_expression_list(struct expression_list *list);

void count_constant_expr(struct expression *expr) { expr->reg_count = 1; }

void count_id_expression(struct expression *expr) {
  id(expr->_id);
  expr->reg_count = 1;
}

void count_index_expression(struct expression *expr) {
  if (expr->_index.object) {
    count_expression(expr->_index.object);
  }

  if (expr->_index.index) {
    count_expression(expr->_index.index);
  }

  // reuse object reg
  expr->reg_count = get_binary_reg_count(expr->_index.object->reg_count,
                                         expr->_index.index->reg_count);
}

void count_func_call(struct expression *expr) {
  if (expr->_call.function_ptr) {
    count_expression(expr->_call.function_ptr);
  }

  if (expr->_call.parameter_list == NULL) {
    expr->reg_count = 1;
    return;
  }

  struct sized_array array = count_expression_list(expr->_call.parameter_list);

  // expression_list should have made space for the function pointer
  array.array[array.size - 1] = expr->_call.function_ptr->reg_count;

  if (array.size == 0 || array.array == NULL) {
    expr->reg_count = 1;
    return;
  }

  expr->reg_count = get_n_ary_reg_count(array.array, array.size);
  free(array.array);
}

void count_postfix_expr(struct expression *expr) {
  if (expr->_unary.base) {
    count_expression(expr->_unary.base);
  }

  // expr->_unary.operator

  // reuse base reg
  expr->reg_count = expr->_unary.base->reg_count;
}

void count_unary_expr(struct expression *expr) {
  // expr->_unary.operator

  if (expr->_unary.base) {
    count_expression(expr->_unary.base);
  }

  // reuse base reg
  expr->reg_count = expr->_unary.base->reg_count;
}

void count_cast_expr(struct expression *expr) {
  if (expr->_cast.type) {
    specifier_list(expr->_cast.type);
  }

  if (expr->_cast.base) {
    count_expression(expr->_cast.base);
  }

  // reuse base reg
  expr->reg_count = expr->_cast.base->reg_count;
}

void count_binary_expr(struct expression *expr) {
  if (expr->_binary.left) {
    count_expression(expr->_binary.left);
  }

  // expr->_binary.operator

  if (expr->_binary.right) {
    count_expression(expr->_binary.right);
  }

  // reuse left reg
  expr->reg_count = get_binary_reg_count(expr->_binary.left->reg_count,
                                         expr->_binary.right->reg_count);
}

void count_ternay_expr(struct expression *expr) {
  if (expr->_ternary.condition) {
    count_expression(expr->_ternary.condition);
  }

  if (expr->_ternary.true_branch) {
    count_expression(expr->_ternary.true_branch);
  }

  if (expr->_ternary.false_branch) {
    count_expression(expr->_ternary.false_branch);
  }

  // reuse condition reg
  expr->reg_count = max(expr->_ternary.condition->reg_count,
                        max(expr->_ternary.true_branch->reg_count,
                            expr->_ternary.false_branch->reg_count));
}

void count_expression(struct expression *expr) {
  if (expr == NULL) {
    return;
  }

  switch (expr->type) {
  case CONST_EXPR:
    count_constant_expr(expr);
    break;
  case ID_EXPR:
    count_id_expression(expr);
    break;
  case INDEX:
    count_index_expression(expr);
    break;
  case FUNC_CALL:
    count_func_call(expr);
    break;
  case POSTFIX:
    count_postfix_expr(expr);
    break;
  case UNARY:
    count_unary_expr(expr);
    break;
  case CAST:
    count_cast_expr(expr);
    break;
  case BINARY:
    count_binary_expr(expr);
    break;
  case TERNARY:
    count_ternay_expr(expr);
    break;
  default:
    CRITICAL("assign", "Unknown expression type");
  }
}

struct sized_array count_expression_list(struct expression_list *list) {
  struct sized_array result = {NULL, 0};

  if (list == NULL) {
    return result;
  }

  struct expression *cur = list->head;

  if (cur == NULL) {
    return result;
  }

  while (cur != NULL) {
    count_expression(cur);
    result.size++;
    cur = cur->next;
  }

  result.size++; // for the function ptr result.
  result.array = malloc(result.size * sizeof(uint64_t));

  cur = list->head;
  size_t i = 0;
  while (cur != NULL) {
    count_expression(cur);
    result.array[i++] = cur->reg_count;
    cur = cur->next;
  }

  return result;
}

uint64_t assign_expression_list(struct expression_list *list);

void assign_constant_expr(struct expression *expr) { expr->reg = next_reg; }

void assign_id_expression(struct expression *expr) { expr->reg = next_reg; }

void assign_index_expression(struct expression *expr) {
  uint64_t object_count =
      expr->_index.object ? expr->_index.object->reg_count : 0;
  uint64_t index_count = expr->_index.index ? expr->_index.index->reg_count : 0;

  if (!expr->_index.object || !expr->_index.index) {
    CRITICAL("assign", "Index expression has no object or no index");
  }

  if (object_count >= index_count) {
    assign_expression(expr->_index.object);

    next_reg++;
    assign_expression(expr->_index.index);
    next_reg--;

    // reuse object reg
    expr->reg = expr->_index.object->reg;
  } else {
    assign_expression(expr->_index.index);

    next_reg++;
    assign_expression(expr->_index.object);
    next_reg--;

    // reuse index reg
    expr->reg = expr->_index.index->reg;
  }
}

void assign_func_call(struct expression *expr) {
  // Edge case: function call with no parameters
  if (expr->_call.parameter_list == NULL) {
    assign_expression(expr->_call.function_ptr);
    return;
  }

  // prepend function pointer to a parameter list
  struct expression_list list;
  list.head = expr->_call.function_ptr;
  list.tail = expr->_call.parameter_list->tail;

  expr->_call.function_ptr->next = expr->_call.parameter_list->head;
  uint64_t reg = assign_expression_list(&list);

  expr->_call.parameter_list->head = list.head;
  expr->_call.parameter_list->tail = list.tail;

  expr->reg = reg;
}

void assign_postfix_expr(struct expression *expr) {
  if (expr->_unary.base) {
    assign_expression(expr->_unary.base);
  }

  // expr->_unary.operator

  // reuse base reg
  expr->reg = expr->_unary.base->reg;
}

void assign_unary_expr(struct expression *expr) {
  // expr->_unary.operator

  if (expr->_unary.base) {
    assign_expression(expr->_unary.base);
  }

  // reuse base reg
  expr->reg = expr->_unary.base->reg;
}

void assign_cast_expr(struct expression *expr) {
  if (expr->_cast.type) {
    specifier_list(expr->_cast.type);
  }

  if (expr->_cast.base) {
    assign_expression(expr->_cast.base);
  }

  // reuse base reg
  expr->reg = expr->_cast.base->reg;
}

void assign_binary_expr(struct expression *expr) {
  uint64_t left_count = expr->_binary.left ? expr->_binary.left->reg_count : 0;
  uint64_t right_count =
      expr->_binary.right ? expr->_binary.right->reg_count : 0;

  if (!expr->_binary.left || !expr->_binary.right) {
    CRITICAL("assign",
             "Binary expression has no left expression or no right expression");
  }

  if (left_count >= right_count) {
    assign_expression(expr->_binary.left);

    next_reg++;
    assign_expression(expr->_binary.right);
    next_reg--;

    // reuse left reg
    expr->reg = expr->_binary.left->reg;
  } else {
    assign_expression(expr->_binary.right);

    next_reg++;
    assign_expression(expr->_binary.left);
    next_reg--;

    // reuse right reg
    expr->reg = expr->_binary.right->reg;
  }
}

void assign_ternay_expr(struct expression *expr) {
  if (expr->_ternary.condition) {
    assign_expression(expr->_ternary.condition);
  }

  if (expr->_ternary.true_branch) {
    assign_expression(expr->_ternary.true_branch);
  }

  if (expr->_ternary.false_branch) {
    assign_expression(expr->_ternary.false_branch);
  }

  // reuse condition reg
  expr->reg = expr->_ternary.condition->reg;
}

void assign_expression(struct expression *expr) {
  if (expr == NULL) {
    return;
  }

  switch (expr->type) {
  case CONST_EXPR:
    assign_constant_expr(expr);
    break;
  case ID_EXPR:
    assign_id_expression(expr);
    break;
  case INDEX:
    assign_index_expression(expr);
    break;
  case FUNC_CALL:
    assign_func_call(expr);
    break;
  case POSTFIX:
    assign_postfix_expr(expr);
    break;
  case UNARY:
    assign_unary_expr(expr);
    break;
  case CAST:
    assign_cast_expr(expr);
    break;
  case BINARY:
    assign_binary_expr(expr);
    break;
  case TERNARY:
    assign_ternay_expr(expr);
    break;
  default:
    CRITICAL("assign", "Unknown expression type");
  }
}

void reverse_sort_expression_list(struct expression_list *list) {
  // create new expression list
  struct expression *head = NULL;
  struct expression *tail = NULL;

  while (list->head) {
    // iterate through the list find max reg count
    struct expression *cur = list->head;
    struct expression *prev = NULL;
    struct expression *prev_of_max = NULL;
    uint64_t max_reg_count = 0;
    for (; cur != NULL; cur = cur->next) {
      if (cur->reg_count > max_reg_count) {
        max_reg_count = cur->reg_count;
        prev_of_max = prev;
      }

      prev = cur;
    }

    cur = prev_of_max ? prev_of_max->next : list->head;

    // remove max reg count from list
    if (prev_of_max) {
      prev_of_max->next = prev_of_max->next->next;
    } else {
      list->head = list->head->next;
    }

    // append max reg count to new list
    if (head == NULL) {
      head = cur;
    } else {
      tail->next = cur;
    }

    tail = cur;
  }

  list->head = head;
  list->tail = tail;
}

uint64_t assign_expression_list(struct expression_list *list) {
  if (list == NULL) {
    CRITICAL("assign", "Try to assign registers to null expression list");
  }

  reverse_sort_expression_list(list);
  struct expression *cur = list->head;

  if (cur == NULL) {
    CRITICAL("assign", "Try to assign registers to empty expression list");
  }

  uint64_t start_reg = next_reg;

  while (cur != NULL) {
    assign_expression(cur);
    next_reg++;
    cur = cur->next;
  }

  next_reg = start_reg;
  return list->head->reg;
}

void expression(struct expression *expr) {
  count_expression(expr);
  assign_expression(expr);
}
