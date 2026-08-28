#ifndef NUM_DEFS_H_
#define NUM_DEFS_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <float.h>

// Unsigned Integers
typedef uint8_t  u8;
#define U8_MIN ((u8) 0)
#define U8_MAX UINT8_MAX

typedef uint16_t u16;
#define U16_MIN ((u16) 0)
#define U16_MAX UINT16_MAX

typedef uint32_t u32;
#define U32_MIN ((u32) 0)
#define U32_MAX UINT32_MAX

typedef uint64_t u64;
#define U64_MIN ((u64) 0)
#define U64_MAX UINT64_MAX

// Signed Integers
typedef  int8_t  s8;
#define S8_MIN INT8_MIN
#define S8_MAX INT8_MAX

typedef  int16_t s16;
#define S16_MIN INT16_MIN
#define S16_MAX INT16_MAX

typedef  int32_t s32;
#define S32_MIN INT32_MIN
#define S32_MAX INT32_MAX

typedef  int64_t s64;
#define S64_MIN INT64_MIN
#define S64_MAX INT64_MAX

// Floats
#ifndef FLT_MAX
  #define FLT_MAX 3.40282347e+38F
#endif
typedef float  f32;
#define F32_MIN (-FLT_MAX)
#define F32_MAX FLT_MAX

#ifndef DBL_MAX
  #define DBL_MAX 1.7976931348623157e+308
#endif
typedef double f64;
#define F64_MIN (-DBL_MAX)
#define F64_MAX DBL_MAX

// Boolean
typedef bool b32;
#define B32_MIN false
#define B32_MAX true

// Pointer types
/// Unsigned Pointer
typedef uintptr_t uptr;
/// Signed Pointer
typedef intptr_t sptr;
/// Memory Measure
typedef size_t umm;

// Fast and Least types
typedef uint_fast8_t  u8_fast;  // Smallest type at least 8 bits, optimized for speed
typedef uint_least16_t u16_least; // Smallest type that is at least 16 bits

// Atomic types
typedef _Atomic u32 au32;
typedef _Atomic u64 au64;

// Handy for memory calculations
#define KILOBYTES(val) ((val) * 1024ULL)
#define MEGABYTES(val) (KILOBYTES(val) * 1024ULL)
#define GIGABYTES(val) (MEGABYTES(val) * 1024ULL)

#endif // NUM_DEFS_H_
