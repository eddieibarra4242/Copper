#include "ast.h"

typedef struct translation_unit *AST;

struct id *create_id(Token *name);
struct declaration *create_declaration(struct id *identifier);
struct translation_unit *create_translation_unit(struct declaration *first);
struct translation_unit *
append_external_declaration(struct translation_unit *list,
                            struct declaration *new_elem);

AST get_tree();
void destroy_ast();
