#include "x86.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../log.h"

#define PIPE_READ 0
#define PIPE_WRITE 1

FILE *assembly_file = NULL;
FILE *stdout_file = NULL;
int as_pid = -1;

const char *byte_register_lookup[] = {
  "%al", "%bl", "%cl", "%dl", "%sil", "%dil", "%r8b",
  "%r9b",  "%r10b", "%r11b", "%r12b", "%r13b", "%r14b", "%r15b",
};

const char *register_lookup[] = {
  "%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi", "%r8",
  "%r9",  "%r10", "%r11", "%r12", "%r13", "%r14", "%r15",
};

#define NUM_REGISTERS (sizeof(register_lookup) / sizeof(register_lookup[0]))

void open_output_file() {
  int assembly_pipe[2];
  int stdout_pipe[2];

  TRY(pipe(assembly_pipe));
  TRY(pipe(stdout_pipe));

  as_pid = fork();
  TRY(as_pid);

  if (as_pid == 0) {
    close(stdout_pipe[PIPE_READ]);
    close(assembly_pipe[PIPE_WRITE]);

    dup2(assembly_pipe[PIPE_READ], STDIN_FILENO);
    dup2(stdout_pipe[PIPE_WRITE], STDOUT_FILENO);
    dup2(stdout_pipe[PIPE_WRITE], STDERR_FILENO);

    close(stdout_pipe[PIPE_WRITE]);
    close(assembly_pipe[PIPE_READ]);

    // For now, output file is always a.out
    execl("/usr/bin/as", "as", "-o", "a.out", NULL);
    CRITICAL("x86", "Failed to execute assembler");
  }

  close(stdout_pipe[PIPE_WRITE]);
  close(assembly_pipe[PIPE_READ]);

  assembly_file = fdopen(assembly_pipe[PIPE_WRITE], "w");
  if (!assembly_file) {
    CRITICAL("x86", "Failed to open assembly file stream");
  }

  stdout_file = fdopen(stdout_pipe[PIPE_READ], "r");
  if (!stdout_file) {
    CRITICAL("x86", "Failed to open stdout file stream");
  }
}

void emit(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(assembly_file, fmt, args);
  va_end(args);
}

void close_output_file() {
  char buffer[1024];
  int status;
  char *result;

  fclose(assembly_file);

  TRY(waitpid(as_pid, &status, 0));

  if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
    WARNV("x86", "Assembler exited with non-zero status: %d",
          WEXITSTATUS(status));
  }

  do {
    result = fgets(buffer, sizeof(buffer), stdout_file);
    if (!result)
      break;

    size_t length = strlen(result);
    if (length > 0 && result[length - 1] == '\n') {
      result[--length] = '\0'; // Remove trailing newline
    }

    if (length > 0 && result[length - 1] == '\r') {
      result[--length] = '\0'; // Remove trailing newline
    }

    INFO("as", "%s", buffer);
  } while (result != NULL);

  fclose(stdout_file);
}

const char *transform_operand(char *str, Operand *op) {
  if (op == NULL) {
    ERROR("x86", "Operand is NULL");
  }

  switch (op->type) {
  case OPERAND_REGISTER:
    if (op->value.reg >= NUM_REGISTERS) {
      ERRORV("x86", "Invalid register number: %lu", op->value.reg);
    }

    return register_lookup[op->value.reg];
  case OPERAND_MEMORY:
    CRITICAL("x86", "Memory operand transformation not implemented");
    break;
  case OPERAND_CONSTANT:
    snprintf(str, 256, "$%ld", op->value.constant);
    return str;
  case OPERAND_LABEL:
    snprintf(str, 256, "%s", op->value.label);
    return str;
  case OPERAND_NONE:
    return "";
  default:
    ERRORV("x86", "Unknown operand type %d", op->type);
  }

  return "";
}

const char *get_byte_register(Operand *op) {
  if (op == NULL) {
    ERROR("x86", "Operand is NULL");
  }

  if (op->type != OPERAND_REGISTER) {
    ERRORV("x86", "Expected register operand, got type %d", op->type);
  }

  if (op->value.reg >= NUM_REGISTERS) {
    ERRORV("x86", "Invalid register number: %lu", op->value.reg);
  }

  return byte_register_lookup[op->value.reg];
}

void emit_x86_instruction(Instruction *insn) {
  if (insn == NULL) {
    ERROR("x86", "Instruction is NULL");
  }

  char dest_str[256] = {0};
  char src1_str[256] = {0};
  char src2_str[256] = {0};

  const char *dest = transform_operand(dest_str, &insn->dest);
  const char *src1 = transform_operand(src1_str, &insn->src1);
  const char *src2 = transform_operand(src2_str, &insn->src2);

  switch (insn->opcode) {
  case ILABEL:
    emit("%s:\n", dest);
    return;
  case ADD:
    emit("addq %s, %s\n", src2, src1);
    emit("movq %s, %s\n", src1, dest);
    return;
  case SUBTRACT:
    emit("subq %s, %s\n", src2, src1);
    emit("movq %s, %s\n", src1, dest);
    return;
  case MULTIPLY:
    emit("imulq %s, %s\n", src2, src1);
    emit("movq %s, %s\n", src1, dest);
    return;
  case DIVIDE:
  case MODULO:
    emit("pushq %%rax\n");
    emit("pushq %%rdx\n");
    emit("xorq %%rdx, %%rdx\n"); // Clear rdx for division
    emit("movq %s, %%rax\n", src1);
    emit("idivq %s\n", src2);

    if (insn->opcode == DIVIDE) {
      emit("movq %%rax, %s\n", dest);
    } else {
      emit("movq %%rdx, %s\n", dest); // For modulo, result is in rdx
    }

    emit("popq %%rdx\n");
    emit("popq %%rax\n");
    return;
  case BITWISE_AND:
  case LOGICAL_AND:
    emit("andq %s, %s\n", src2, src1);
    emit("movq %s, %s\n", src1, dest);
    return;
  case BITWISE_OR:
  case LOGICAL_OR:
    emit("orq %s, %s\n", src2, src1);
    emit("movq %s, %s\n", src1, dest);
    return;
  case BITWISE_XOR:
    emit("xorq %s, %s\n", src2, src1);
    emit("movq %s, %s\n", src1, dest);
    return;
  case BITWISE_NOT:
    emit("not %s\n", src1);
    emit("movq %s, %s\n", src1, dest);
    return;
  case LOGICAL_NOT:
    emit("test %s, %s\n", src1, src1); // Set zero flag if src1 is zero
    emit("setne %s\n", get_byte_register(&insn->dest)); // Set dest to 1 if src1 is not zero, else set to 0
    emit("neg %s\n", dest); // Negate dest to get 0 if 0, -1 if 1
    emit("addq $1, %s\n", dest); // Add 1 to dest to logical not
                                // combined with the negation, 0 becomes 1, and 1 becomes 0
    return;
  case LEFT_SHIFT:
    emit("shlq %s, %s\n", src2, src1);
    emit("movq %s, %s\n", src1, dest);
    return;
  case RIGHT_SHIFT:
    emit("shrq %s, %s\n", src2, src1);
    emit("movq %s, %s\n", src1, dest);
    return;
  case EQUAL:
    emit("cmp %s, %s\n", src2, src1);
    emit("sete %s\n", dest);
    return;
  case NOT_EQUAL:
    emit("cmp %s, %s\n", src2, src1);
    emit("setne %s\n", dest);
    return;
  case LESS_THAN:
    emit("cmp %s, %s\n", src2, src1);
    emit("setl %s\n", dest);
    return;
  case GREATER_THAN:
    emit("cmp %s, %s\n", src2, src1);
    emit("setg %s\n", dest);
    return;
  case LESS_EQUAL:
    emit("cmp %s, %s\n", src2, src1);
    emit("setle %s\n", dest);
    return;
  case GREATER_EQUAL:
    emit("cmp %s, %s\n", src2, src1);
    emit("setge %s\n", dest);
    return;
  case JUMP:
    emit("jmp %s\n", dest);
    return;
  case JUMP_NOT_ZERO:
    emit("jnz %s\n", dest);
    return;
  case JUMP_EQU_ZERO:
    emit("jz %s\n", dest);
    return;
  case CALL:
    emit("call %s\n", dest);
    return;
  case IRETURN:
    emit("ret\n");
    return;
  case LOAD_CONST:
  case MOVE:
    emit("movq %s, %s\n", src1, dest);
    return;
  default:
    ERRORV("x86", "Unknown instruction opcode: %d", insn->opcode);
  }
}

void x86_output_insns(InstructionList *insns) {
  open_output_file();

  emit(".globl _start\n");
  emit(".globl main\n");
  emit(".section .text\n");
  for (Instruction *cur = insns->head; cur != NULL; cur = cur->next) {
    emit_x86_instruction(cur);
  }

  emit("_start:\n"
       "  call main\n"
       "  mov %%rax, %%rdi\n" // Move return value to rdi
       "  mov $60, %%rax\n"   // syscall: exit
       "  syscall\n");
  emit("\n");

  close_output_file();
}
