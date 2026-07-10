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
#define ITERATIONS 2000
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
typedef int16_t ee_s16;
typedef uint16_t ee_u16;
typedef int32_t ee_s32;
#if HAS_FLOAT
typedef float ee_f32;
typedef float ee_f16;
#else
typedef int32_t ee_f32;
typedef int32_t ee_f16;
#endif
typedef uint8_t ee_u8;
typedef uint32_t ee_u32;
typedef uintptr_t ee_ptr_int;
typedef uint32_t ee_size_t;
#if HAS_FLOAT
#ifdef __cplusplus
#include <cmath>
#else
#include <math.h>
#endif
#ifdef matrix_big
#undef matrix_big
#endif
#define matrix_big(x) ((ee_f32)(61440.0f + (x)))
#ifdef matrix_clip
#undef matrix_clip
#endif
#define matrix_clip(x, y)                                  \
  ((ee_f32)((x) - (int)((x) / ((y) ? 256.0f : 65536.0f)) * \
                      ((y) ? 256.0f : 65536.0f)))
#ifdef bit_extract
#undef bit_extract
#endif
#define bit_extract(x, from, to)                                       \
  ((ee_f32)(((x) / (float)(1 << (from))) -                             \
            (int)(((x) / (float)(1 << (from))) / (float)(1 << (to))) * \
                (float)(1 << (to))))
#endif
static inline void* align_mem(void* x) {
  return (void*)(4 + (((uintptr_t)x - 1) & ~3));
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
#endif /* CORE_PORTME_H */
