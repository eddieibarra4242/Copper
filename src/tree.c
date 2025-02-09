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

  decl->specifiers = specifiers;
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

// Destroying functions

void destroy_id(struct id *id) { free(id); }

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

void destroy_declaration(struct declaration *decl) {
  destroy_specifiers(decl->specifiers);
  destroy_id(decl->name);
  free(decl);
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

void sense_id(struct id *id) { delete_allocation_entry(id); }

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

void sense_declaration(struct declaration *decl) {
  sense_specifiers(decl->specifiers);
  sense_id(decl->name);
  delete_allocation_entry(decl);
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

  DEBUG("Freed %zu unused blocks", unused_allocations_count);
}

void destroy_ast() {
  if (root)
    destroy_translation_unit(root);
}
