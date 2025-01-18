#ifndef MINMAX_H
#define MINMAX_H

#include <stdint.h>

#define minm(a, b) ((a) < (b) ? (a) : (b))
#define maxm(a, b) ((a) > (b) ? (a) : (b))
#define clampm(v, low, high) minm(maxm(v, low), high)

static inline uint32_t minu32(uint32_t a, uint32_t b) { return minm(a, b); }
static inline uint32_t maxu32(uint32_t a, uint32_t b) { return maxm(a, b); }
static inline uint32_t clampu32(uint32_t v, uint32_t low, uint32_t high) { return clampm(v, low, high); }

static inline int32_t min32(int32_t a, int32_t b) { return minm(a, b); }
static inline int32_t max32(int32_t a, int32_t b) { return maxm(a, b); }
static inline int32_t clamp32(int32_t v, int32_t low, int32_t high) { return clampm(v, low, high); }

static inline uint16_t minu16(uint16_t a, uint16_t b) { return minm(a, b); }
static inline uint16_t maxu16(uint16_t a, uint16_t b) { return maxm(a, b); }
static inline uint16_t clampu16(uint16_t v, uint16_t low, uint16_t high) { return clampm(v, low, high); }

static inline int16_t min16(int16_t a, int16_t b) { return minm(a, b); }
static inline int16_t max16(int16_t a, int16_t b) { return maxm(a, b); }
static inline int16_t clamp16(int16_t v, int16_t low, int16_t high) { return clampm(v, low, high); }

static inline uint8_t minu8(uint8_t a, uint8_t b) { return minm(a, b); }
static inline uint8_t maxu8(uint8_t a, uint8_t b) { return maxm(a, b); }
static inline uint8_t clampu8(uint8_t v, uint8_t low, uint8_t high) { return clampm(v, low, high); }

static inline int8_t min8(int8_t a, int8_t b) { return minm(a, b); }
static inline int8_t max8(int8_t a, int8_t b) { return maxm(a, b); }
static inline int8_t clamp8(int8_t v, int8_t low, int8_t high) { return clampm(v, low, high); }

#endif
