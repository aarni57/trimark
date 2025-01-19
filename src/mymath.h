#ifndef MYMATH_H
#define MYMATH_H

#include <stdint.h>
#include <assert.h>

static inline uint32_t clz_u32(uint32_t a) {
    assert(a != 0);
#if defined(__WATCOMC__)
    __asm {
        .386
        mov ecx, a
        bsr eax, ecx
        mov ebx, 31
        sub ebx, eax
        mov a, ebx
    }

    return a;
#else
    uint32_t r = 32;
    if (a >= 0x00010000) { a >>= 16; r -= 16; }
    if (a >= 0x00000100) { a >>=  8; r -=  8; }
    if (a >= 0x00000010) { a >>=  4; r -=  4; }
    if (a >= 0x00000004) { a >>=  2; r -=  2; }
    r -= a - (a & (a >> 1));
    return r;
#endif
}

static inline uint32_t mul32(uint32_t x, uint32_t y) {
#if defined(__WATCOMC__)
    __asm {
        .386
        mov eax, x
        mov ebx, y
        mul ebx
        mov x, eax
    }

    return x;
#else
    return x * y;
#endif
}

static inline int32_t imul32(int32_t x, int32_t y) {
#if defined(__WATCOMC__)
    __asm {
        .386
        mov eax, x
        mov ebx, y
        imul ebx
        mov x, eax
    }

    return x;
#else
    return x * y;
#endif
}

static inline uint32_t div32(uint32_t x, uint32_t y) {
#if defined(__WATCOMC__)
    __asm {
        .386
        mov eax, x
        mov ebx, y
        xor edx, edx
        div ebx
        mov x, eax
    }

    return x;
#else
    return x / y;
#endif
}

#endif
