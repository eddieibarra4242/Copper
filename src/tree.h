#include "ast.h"

typedef struct translation_unit *AST;

struct id *create_id(Token *name);
struct declaration *create_declaration(struct specifier_list *specifiers,
                                       struct id *identifier);
struct declaration *
create_init_declaration(struct specifier_list *specifiers,
                        struct init_declarator_list *init_declarator_list);
struct declaration *create_function(struct specifier_list *specifiers,
                                    struct id *identifier,
                                    struct statement *body);
struct declaration *create_type_definition(struct specifier_list *specifiers,
                                           struct id *identifier);
struct translation_unit *create_translation_unit(struct declaration *first);
struct translation_unit *
append_external_declaration(struct translation_unit *list,
                            struct declaration *new_elem);

struct specifier *create_token_specifier(Token *token);
struct specifier *create_id_specifier(struct id *id);

struct specifier_list *create_specifier_list(struct specifier *tail);
struct specifier_list *prepend_specifier(struct specifier *prefix,
                                         struct specifier_list *list);

struct statement *create_break_stmt();
struct statement *create_compound_stmt(struct statement_list *list);
struct statement *create_continue_stmt();
struct statement *create_decl_stmt(struct declaration *decl);
struct statement *create_expr_stmt(struct expression *expr);
struct statement *create_for_stmt_with_decl(struct declaration *decl,
                                            struct expression *condition,
                                            struct expression *step_expression,
                                            struct statement *body);
struct statement *create_for_stmt_with_expr(struct expression *init,
                                            struct expression *condition,
                                            struct expression *step_expression,
                                            struct statement *body);
struct statement *create_goto_stmt(struct id *label);
struct statement *create_if_stmt(struct expression *condition,
                                 struct statement *body,
                                 struct statement *else_body);
struct statement *create_label_stmt(struct id *name);
struct statement *create_return_stmt(struct expression *expr);
struct statement *create_switch_stmt(struct expression *condition,
                                     struct statement *body);
struct statement *create_case_stmt(struct expression *test);
struct statement *create_default_stmt();
struct statement *create_do_while_stmt(struct statement *body,
                                       struct expression *condition);
struct statement *create_while_stmt(struct expression *condition,
                                    struct statement *body);

struct statement_list *create_stmt_list(struct statement *first);
struct statement_list *prepend_stmt(struct statement *new_stmt,
                                    struct statement_list *list);
struct statement_list *append_stmt(struct statement_list *list,
                                   struct statement *new_stmt);

struct expression *create_id_expression(struct id *id);
struct expression *create_const_expression(Token *constant);
struct expression *create_dot_index_expression(struct expression *obj,
                                               struct expression *index);
struct expression *create_arrow_index_expression(struct expression *obj,
                                                 struct expression *index);
struct expression *create_array_index_expression(struct expression *obj,
                                                 struct expression *index);
struct expression *
create_call_expression(struct expression *function_ptr,
                       struct expression_list *parameter_list);
struct expression *create_postfix_expression(struct expression *base,
                                             Token *operator);
struct expression *create_unary_expression(Token *operator,
                                           struct expression * base);
struct expression *create_cast_expression(struct specifier_list *type,
                                          struct expression *base);
struct expression *create_binary_expression(struct expression *left,
                                            Token *operator,
                                            struct expression * right);
struct expression *create_ternary_expression(struct expression *condition,
                                             struct expression *true_branch,
                                             struct expression *false_branch);

struct expression_list *create_expr_list(struct expression *first_elem);
struct expression_list *append_expr(struct expression_list *list,
                                    struct expression *expr);

struct initialized_declarator *
create_initialized_declarator(struct id *declarator,
                              struct expression *initializer);
struct init_declarator_list *
create_init_declarator_list(struct initialized_declarator *first_elem);
struct init_declarator_list *
append_initialized_declarator(struct init_declarator_list *list,
                              struct initialized_declarator *new_elem);

AST get_tree();
void free_unused_parse_branches();
void destroy_ast();
