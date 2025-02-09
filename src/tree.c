#include "tree.h"
#include "log.h"

#define NEW(ty) allocate_or_error(sizeof(ty))

AST root = NULL;

void *allocate_or_error(size_t size) {
  void *result = malloc(size);

  if (result == NULL) {
    CRITICAL("ast", "Out of memory!");
  }

  return result;
}

struct id *create_id(Token *name) {
  struct id *result = NEW(struct id);

  result->name = name;

  return result;
}

struct declaration *create_declaration(struct id *identifier) {
  struct declaration *decl = NEW(struct declaration);

  decl->name = identifier;
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

void destroy_id(struct id *id) { free(id); }

void destory_declaration(struct declaration *decl) {
  destroy_id(decl->name);
  free(decl);
}

void destroy_translation_unit(struct translation_unit *unit) {
  struct declaration *cur = unit->external_declarations.head;
  struct declaration *next = NULL;

  while (cur) {
    next = cur->next;
    destory_declaration(cur);
    cur = next;
  }

  free(unit);
}

AST get_tree() { return root; }

void destroy_ast() {
  if (root)
    destroy_translation_unit(root);
}
