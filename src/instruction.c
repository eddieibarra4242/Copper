#include "instruction.h"

#include <stdlib.h>

#include "log.h"

InstructionList *create_instruction_list(void) {
  InstructionList *list = malloc(sizeof(InstructionList));

  if (list == NULL) {
    CRITICAL("insn", "Out of memory!");
  }

  list->head = NULL;
  list->tail = NULL;
  return list;
}

Instruction *append_instruction(InstructionList *list, enum InstructionSet opcode,
                                         Operand dest, Operand src1, Operand src2) {
  if (list == NULL) {
    CRITICAL("insn", "Cannot append instruction to a NULL list");
  }

  Instruction *insn = malloc(sizeof(Instruction));

  if (insn == NULL) {
    CRITICAL("insn", "Out of memory!");
  }

  insn->opcode = opcode;
  insn->dest = dest;
  insn->src1 = src1;
  insn->src2 = src2;
  insn->next = NULL;

  if (list->head == NULL) {
    list->head = insn;
    list->tail = insn;
  } else {
    list->tail->next = insn;
    list->tail = insn;
  }

  return insn;
}

InstructionList *append_instruction_list(InstructionList *dest,
                                                   InstructionList *appendage) {
  if (dest == NULL || appendage == NULL) {
    CRITICAL("insn", "Cannot append NULL instruction list");
  }

  if (appendage->head == NULL) {
    return dest; // Nothing to append
  }

  if (dest->head == NULL) {
    dest->head = appendage->head;
    dest->tail = appendage->tail;
  } else {
    dest->tail->next = appendage->head;
    dest->tail = appendage->tail;
  }

  return dest;
}

void free_instruction_list(InstructionList *list) {
  if (list == NULL) {
    return; // Nothing to free
  }

  // Delibrately not freeing the instructions themselves,
  // as they may still be in use by the program.

  free(list);
}

void destroy_instruction_list(InstructionList *list) {
  if (list == NULL) {
    return; // Nothing to free
  }

  Instruction *current = list->head;
  Instruction *next;

  while (current != NULL) {
    next = current->next;
    free(current);
    current = next;
  }

  free(list);
}
