// Copyright 2026 Google LLC
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef CORE_PORTME_H
#define CORE_PORTME_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifndef HAS_FLOAT
#define HAS_FLOAT 1
#endif
#define HAS_TIME_H 1
#define USE_CLOCK 1
#ifndef HAS_STDIO
#define HAS_STDIO 1
#endif
#ifndef HAS_PRINTF
#define HAS_PRINTF 1
#endif

#define COMPILER_VERSION "GCC"

// ADR 009, ADR 013: Centralized Execution Workload Configuration (10s-100s run)
#ifndef ITERATIONS
#define ITERATIONS 1000
#endif

#ifndef COMPILER_FLAGS
#define COMPILER_FLAGS \
  "-march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f -O3 -ftree-vectorize"
#endif

#define MEM_LOCATION "STACK"

typedef int8_t ee_s8;
typedef uint8_t ee_u8;
typedef int16_t ee_s16;
typedef uint16_t ee_u16;
typedef int32_t ee_s32;
typedef float ee_f32;
typedef float ee_f16;
typedef uint32_t ee_u32;
typedef uintptr_t ee_ptr_int;
typedef size_t ee_size_t;

#if HAS_FLOAT
#include <math.h>

// ADR 001: Float-Int Cast translation layer using type-punning unions
typedef union {
  ee_f32 f;
  ee_u32 i;
} FloatIntUnion;

static inline ee_f32 matrix_big_override(ee_f32 x) {
  FloatIntUnion u;
  u.f = x;
  u.i |= 0xf000;
  return u.f;
}

static inline ee_f32 matrix_clip_override(ee_f32 x, ee_u32 y) {
  FloatIntUnion u;
  u.f = x;
  u.i &= y;
  return u.f;
}

static inline ee_u32 bit_extract_override(ee_f32 x, int from, int to) {
  FloatIntUnion u;
  u.f = x;
  return (u.i >> from) & ((1U << to) - 1);
}

// Redefine core benchmark algorithmic macros (ADR 018)
#define matrix_big(x) matrix_big_override(x)
#define matrix_clip(x, y) matrix_clip_override(x, y)
#define bit_extract(x, from, to) bit_extract_override(x, from, to)

#endif

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

#define STATIC_MEMBLOCK_SIZE (16 * 1024 * 1024)

void portable_init(core_portable* p, int* argc, char* argv[]);
void portable_fini(core_portable* p);
void* portable_malloc(size_t size);
void portable_free(void* p);
void reset_portable_malloc();

#define PERFORMANCE_RUN 1

#ifndef EE_TICK_RESOLUTION
#define EE_TICK_RESOLUTION 36000000
#endif

// ADR 024: Explicit variadic prototype declaration
int ee_printf(const char* fmt, ...);

static inline void* align_mem(void* x) {
  return (void*)(4 + (((uintptr_t)x - 1) & ~3));
}

#endif /* CORE_PORTME_H */
