#pragma once

#ifdef CU_USE_X86
#  include "backends/x86.h"
#  define output_insns(insns) x86_output_insns(insns)
#else
#  include "log.h"
#  define output_insns(insns) ERROR("backend", "No ISA selected!")
#endif