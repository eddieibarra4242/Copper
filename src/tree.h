#include "ast.h"

typedef struct translation_unit *AST;

struct id *create_id(Token *name);
struct declaration *create_declaration(struct specifier_list *specifiers,
                                       struct id *identifier);
struct declaration *create_function(struct specifier_list *specifiers,
                                    struct id *identifier,
                                    struct statement *body);
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
struct statement *create_expr_stmt();
struct statement *create_for_stmt(struct declaration *decl,
                                  struct statement *body);
struct statement *create_goto_stmt(struct id *label);
struct statement *create_if_stmt(struct statement *body,
                                 struct statement *else_body);
struct statement *create_label_stmt(struct id *name);
struct statement *create_return_stmt();
struct statement *create_switch_stmt(struct statement *body);
struct statement *create_case_stmt();
struct statement *create_default_stmt();
struct statement *create_do_while_stmt(struct statement *body);
struct statement *create_while_stmt(struct statement *body);

struct statement_list *create_stmt_list(struct statement *first);
struct statement_list *prepend_stmt(struct statement *new_stmt,
                                    struct statement_list *list);
struct statement_list *append_stmt(struct statement_list *list,
                                   struct statement *new_stmt);

AST get_tree();
void free_unused_parse_branches();
void destroy_ast();
