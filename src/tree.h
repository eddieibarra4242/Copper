#include "ast.h"

typedef struct translation_unit *AST;

struct id *create_id(Token *name);
struct declaration *create_declaration(struct specifier_list *specifiers,
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

AST get_tree();
void free_unused_parse_branches();
void destroy_ast();
