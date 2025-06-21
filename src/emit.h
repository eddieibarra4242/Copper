#pragma once

#include "instruction.h"

/**
 * Translates the AST to an instruction list.
 */
InstructionList *emit_intermediate_representation(void);
