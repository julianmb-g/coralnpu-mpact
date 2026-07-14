#ifndef CORE_PORTME_H
#define CORE_PORTME_H

#ifndef HAS_FLOAT
#define HAS_FLOAT 1
#endif
#define HAS_TIME_H 1
#define USE_CLOCK 1
#define HAS_STDIO 1
#define HAS_PRINTF 0
#define COMPILER_VERSION "GCC"
#ifndef ITERATIONS
#define ITERATIONS 1000
#endif
#ifndef COMPILER_FLAGS
#define COMPILER_FLAGS FLAGS_STR
#endif
#ifndef FLAGS_STR
#define FLAGS_STR "-march=rv32imf_zve32f_zicsr_zifencei_zbb"
#endif
#define MEM_LOCATION "STACK"

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#if HAS_FLOAT
#ifdef __cplusplus
#include <cmath>
#else
#include <math.h>
#endif

static inline float matrix_big(float x) {
#ifdef __cplusplus
  return static_cast<float>(61440.0f + x);
#else
  return (float)(61440.0f + x);
#endif
}

static inline float matrix_clip(float x, int y) {
  float divisor = y ? 256.0f : 65536.0f;
#ifdef __cplusplus
  return static_cast<float>(x - static_cast<int>(x / divisor) * divisor);
#else
  return (float)(x - (int)(x / divisor) * divisor);
#endif
}

static inline float bit_extract(float x, int from, int to) {
#ifdef __cplusplus
  float divisor1 = static_cast<float>(1 << from);
  float divisor2 = static_cast<float>(1 << to);
  return static_cast<float>(
      (x / divisor1) - static_cast<int>((x / divisor1) / divisor2) * divisor2);
#else
  float divisor1 = (float)(1 << from);
  float divisor2 = (float)(1 << to);
  return (float)((x / divisor1) - (int)((x / divisor1) / divisor2) * divisor2);
#endif
}

#endif

static inline void* align_mem(void* x) {
#ifdef __cplusplus
  return reinterpret_cast<void*>(4 +
                                 ((reinterpret_cast<uintptr_t>(x) - 1) & ~3));
#else
  return (void*)(4 + (((uintptr_t)x - 1) & ~3));
#endif
}
#define CORETIMETYPE uint32_t
typedef uint32_t CORE_TICKS;

#define SEED_METHOD 2
#define MEM_METHOD 0
#define MULTITHREAD 1
#define MAIN_HAS_NOARGC 1
#define MAIN_HAS_NORETURN 0

extern uint32_t default_num_contexts;
typedef struct CORE_PORTABLE_S {
  uint8_t portable_id;
} core_portable;

void portable_init(core_portable* p, int* argc, char* argv[]);
void portable_fini(core_portable* p);
void* portable_malloc(uint32_t size);
void portable_free(void* p);
void reset_portable_malloc();

#define PERFORMANCE_RUN 1

#ifndef EE_TICK_RESOLUTION
#define EE_TICK_RESOLUTION 36000000
#endif

#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif
void ee_printf(const char* fmt, ...);

typedef int8_t ee_s8;
typedef uint8_t ee_u8;
typedef int16_t ee_s16;
typedef uint16_t ee_u16;
typedef int32_t ee_s32;
typedef float ee_f32;
typedef float ee_f16;
typedef uint32_t ee_u32;
typedef uintptr_t ee_ptr_int;
typedef uint32_t ee_size_t;

// Types transitioned to standard <cstdint> types.

#endif /* CORE_PORTME_H */
