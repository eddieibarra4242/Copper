#pragma once

#define UNUSED(x) (void)(x)
#define NELEMS(x) (sizeof(x) / sizeof(x[0]))

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
