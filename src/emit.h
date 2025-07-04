#pragma once

#include "instruction.h"

/**
 * Frees the memory allocated for the generated labels.
 */
void free_generated_labels(void);

/**
 * Translates the AST to an instruction list.
 */
InstructionList *emit_intermediate_representation(void);
