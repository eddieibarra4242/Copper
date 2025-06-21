#include "emit.h"

#include "common.h"
#include "log.h"
#include "tree.h"

// Entry point
InstructionList *emit_intermediate_representation() {
  InstructionList *list = create_instruction_list();
  // TEMP
  append_instruction(list, ILABEL, OP_LABEL("main"), OP_NONE, OP_NONE);
  append_instruction(list, ADD, OP_REG(0), OP_REG(0), OP_REG(1));
  append_instruction(list, SUBTRACT, OP_REG(0), OP_REG(2), OP_REG(1));
  append_instruction(list, MULTIPLY, OP_MEM(0x10bc), OP_REG(2), OP_REG(2));
  append_instruction(list, DIVIDE, OP_MEM(0x10bc), OP_REG(2), OP_CONST(42));
  append_instruction(list, MODULO, OP_MEM(0x10bc), OP_REG(2), OP_CONST(42));
  append_instruction(list, BITWISE_AND, OP_REG(0), OP_REG(0), OP_REG(1));
  append_instruction(list, BITWISE_OR, OP_REG(0), OP_REG(0), OP_REG(1));
  append_instruction(list, BITWISE_XOR, OP_REG(0), OP_REG(0), OP_REG(1));
  append_instruction(list, BITWISE_NOT, OP_REG(0), OP_REG(0), OP_REG(1));
  append_instruction(list, LEFT_SHIFT, OP_REG(0), OP_REG(0), OP_CONST(12));
  append_instruction(list, RIGHT_SHIFT, OP_REG(0), OP_REG(0), OP_CONST(12));
  append_instruction(list, EQUAL, OP_REG(0), OP_REG(0), OP_REG(1));
  append_instruction(list, NOT_EQUAL, OP_REG(0), OP_REG(0), OP_REG(1));
  append_instruction(list, LESS_THAN, OP_REG(0), OP_REG(0), OP_REG(1));
  append_instruction(list, GREATER_THAN, OP_REG(0), OP_REG(0), OP_REG(1));
  append_instruction(list, LESS_EQUAL, OP_REG(0), OP_REG(0), OP_REG(1));
  append_instruction(list, GREATER_EQUAL, OP_REG(0), OP_REG(0), OP_REG(1));
  append_instruction(list, JUMP, OP_LABEL("main"), OP_NONE, OP_NONE);
  append_instruction(list, JUMP_IF_TRUE, OP_LABEL("if"), OP_REG(0), OP_NONE);
  append_instruction(list, JUMP_IF_FALSE, OP_LABEL("else"), OP_REG(0), OP_NONE);
  append_instruction(list, CALL, OP_LABEL("fib"), OP_NONE, OP_NONE);
  append_instruction(list, IRETURN, OP_NONE, OP_NONE, OP_NONE);
  append_instruction(list, LOAD, OP_REG(0), OP_MEM(0x10bc), OP_NONE);
  append_instruction(list, STORE, OP_MEM(0x10bc), OP_REG(0), OP_NONE);
  append_instruction(list, LOAD_CONST, OP_REG(0), OP_CONST(12), OP_NONE);
  append_instruction(list, MOVE, OP_REG(0), OP_REG(1), OP_NONE);

  return list;
}
