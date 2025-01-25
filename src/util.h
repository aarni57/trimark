#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

#define swap32(a, b) { int32_t c = b; b = a; a = c; }

#define is_aligned(ptr, alignment) (((size_t)(ptr) & ((alignment) - 1)) == 0)

#if !defined(NDEBUG)
#   define ONLY_DEBUG(a) a
#else
#   define ONLY_DEBUG(a)
#endif

#endif
