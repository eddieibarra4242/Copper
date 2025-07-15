#pragma once

#include <stdint.h>

// Intermediate representation instruction set.
enum InstructionSet {
  ILABEL, // Label for jump targets

  // Arithmetic operations
  ADD,      // Addition
  SUBTRACT, // Subtraction
  MULTIPLY, // Multiplication
  DIVIDE,   // Division
  MODULO,   // Modulo

  // Bitwise operations
  BITWISE_AND, // Bitwise AND
  BITWISE_OR,  // Bitwise OR
  BITWISE_XOR, // Bitwise XOR
  BITWISE_NOT, // Bitwise NOT
  LEFT_SHIFT,  // Left shift
  RIGHT_SHIFT, // Right shift

  // Logical operations
  LOGICAL_AND, // Logical AND
  LOGICAL_OR,  // Logical OR
  LOGICAL_NOT, // Logical NOT
  LOGICAL_XOR, // Logical XOR

  // Comparison operations
  EQUAL,         // Equal to
  NOT_EQUAL,     // Not equal to
  LESS_THAN,     // Less than
  GREATER_THAN,  // Greater than
  LESS_EQUAL,    // Less than or equal to
  GREATER_EQUAL, // Greater than or equal to

  // Control flow operations
  JUMP,          // Unconditional jump
  JUMP_NOT_ZERO, // Conditional jump if true
  JUMP_EQU_ZERO, // Conditional jump if false

  // Function calls and returns
  CALL,    // Call function
  IRETURN, // Return from function

  // Load and store operations
  LOAD,       // Load value from memory
  STORE,      // Store value to memory
  LOAD_CONST, // Load constant value
  MOVE,       // Move value between registers or memory
};

typedef struct Operand {
  // Type of the operand
  enum {
    OPERAND_REGISTER,
    OPERAND_MEMORY,
    OPERAND_CONSTANT,
    OPERAND_LABEL,
    OPERAND_NONE, // Used for uninitialized or empty operands
  } type;

  union {
    // Register number (if type is REGISTER)
    uint64_t reg;
    // Memory address (if type is MEMORY)
    uint64_t mem_addr;
    // Constant value (if type is CONSTANT)
    uint64_t constant;
    // Label name (if type is LABEL)
    // Note: Label names are typically strings, but here we use a fixed-size
    // char array for simplicity
    const char *label;
  } value;
} Operand;

typedef struct Instruction {
  // The operation code for the instruction
  enum InstructionSet opcode;

  Operand dest;
  Operand src1;
  Operand src2;

  struct Instruction *next;
} Instruction;

typedef struct {
  // Pointer to the first instruction in the list
  Instruction *head;
  // Pointer to the last instruction in the list
  Instruction *tail;
} InstructionList;

#define OP_MEM(loc)                                                            \
  ((Operand){.type = OPERAND_MEMORY, .value.mem_addr = (uint64_t)(loc)})
#define OP_REG(num)                                                            \
  ((Operand){.type = OPERAND_REGISTER, .value.reg = (uint64_t)(num)})
#define OP_CONST(val)                                                          \
  ((Operand){.type = OPERAND_CONSTANT, .value.constant = (uint64_t)(val)})
#define OP_LABEL(name) ((Operand){.type = OPERAND_LABEL, .value.label = (name)})
#define OP_NONE ((Operand){.type = OPERAND_NONE, .value.constant = 0})

/**
 * Creates a new instruction list.
 *
 * @return Pointer to the newly created instruction list.
 */
InstructionList *create_instruction_list();

/**
 * Creates a new instruction with the specified opcode and operands.
 *
 * @param list Pointer to the instruction list where the instruction will be
 * added.
 * @param opcode The operation code for the instruction.
 * @param dest The destination operand.
 * @param src1 The first source operand.
 * @param src2 The second source operand.
 * @return Pointer to the newly created instruction.
 */
Instruction *append_instruction(InstructionList *list,
                                enum InstructionSet opcode, Operand dest,
                                Operand src1, Operand src2);

/**
 * Appends an instruction list to another instruction list.
 *
 * @param dest Pointer to the destination instruction list.
 * @param appendage Pointer to the instruction list to append.
 * @return Pointer to the destination instruction list after appending.
 */
InstructionList *append_instruction_list(InstructionList *dest,
                                         InstructionList *appendage);

/**
 * Frees the memory allocated for an instruction list ONLY.
 * This does not free the instructions themselves.
 *
 * @param list Pointer to the instruction list to be freed.
 */
void free_instruction_list(InstructionList *list);

/**
 * Destroys the instruction list and all its instructions.
 *
 * @param list Pointer to the instruction list to be destroyed.
 */
void destroy_instruction_list(InstructionList *list);
