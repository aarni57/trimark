#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

#define is_aligned(ptr, alignment) (((size_t)ptr & (alignment - 1)) == 0)

#endif
