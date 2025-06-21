#include "debug_insn.h"

#include <stdio.h>

#include "log.h"

char insn_str[4096];
const char* op_code;
char op1_str[256];
char op2_str[256];
char op3_str[256];

void debug_operand(char *str, Operand *op) {
  if (op == NULL) {
    ERROR("debug", "Operand is NULL");
  }

  switch (op->type) {
    case OPERAND_REGISTER:
      snprintf(str, 256, "r%lu", op->value.reg);
      break;
    case OPERAND_MEMORY:
      snprintf(str, 256, "mem[0x%lx]", op->value.mem_addr);
      break;
    case OPERAND_CONSTANT:
      snprintf(str, 256, "%lu", op->value.constant);
      break;
    case OPERAND_LABEL:
      snprintf(str, 256, "%s", op->value.label);
      break;
    case OPERAND_NONE:
      snprintf(str, 256, "(blank)");
      break;
    default:
      ERRORV("debug", "Unknown operand type %d", op->type);
  }
}

void debug_instruction(Instruction *insn) {
  if (insn == NULL) {
    ERROR("debug", "Instruction is NULL");
  }

  switch (insn->opcode) {
    case ILABEL: op_code = "label"; break;
    case ADD: op_code = "add"; break;
    case SUBTRACT: op_code = "sub"; break;
    case MULTIPLY: op_code = "mul"; break;
    case DIVIDE: op_code = "div"; break;
    case MODULO: op_code = "mod"; break;
    case BITWISE_AND: op_code = "and"; break;
    case BITWISE_OR: op_code = "or"; break;
    case BITWISE_XOR: op_code = "eor"; break;
    case BITWISE_NOT: op_code = "not"; break;
    case LEFT_SHIFT: op_code = "shl"; break;
    case RIGHT_SHIFT: op_code = "shr"; break;
    case EQUAL: op_code = "eq"; break;
    case NOT_EQUAL: op_code = "ne"; break;
    case LESS_THAN: op_code = "le"; break;
    case GREATER_THAN: op_code = "gr"; break;
    case LESS_EQUAL: op_code = "leq"; break;
    case GREATER_EQUAL: op_code = "geq"; break;
    case JUMP: op_code = "jmp"; break;
    case JUMP_IF_TRUE: op_code = "jt"; break;
    case JUMP_IF_FALSE: op_code = "jf"; break;
    case CALL: op_code = "call"; break;
    case IRETURN: op_code = "ret"; break;
    case LOAD: op_code = "l"; break;
    case STORE: op_code = "s"; break;
    case LOAD_CONST: op_code = "li"; break;
    case MOVE: op_code = "mv"; break;
    default:
      DEBUG("Instruction: UNKNOWN OPCODE %d", insn->opcode);
  }

  debug_operand(op1_str, &insn->dest);
  debug_operand(op2_str, &insn->src1);
  debug_operand(op3_str, &insn->src2);

  if (insn->opcode == ILABEL) {
    snprintf(insn_str, 4096, "%s:", op1_str);
  } else if (insn->src2.type != OPERAND_NONE) {
    snprintf(insn_str, 4096, "  %s %s, %s, %s", op_code, op1_str, op2_str, op3_str);
  } else if (insn->src1.type != OPERAND_NONE) {
    snprintf(insn_str, 4096, "  %s %s, %s", op_code, op1_str, op2_str);
  } else if (insn->dest.type != OPERAND_NONE) {
    snprintf(insn_str, 4096, "  %s %s", op_code, op1_str);
  } else {
    snprintf(insn_str, 4096, "  %s", op_code);
  }

  DEBUG("%s", insn_str);
}

void debug_insns(InstructionList *list) {
  if (list == NULL)
    return;

  Instruction *cur = list->head;
  for (; cur != NULL; cur = cur->next) {
    debug_instruction(cur);
  }
}