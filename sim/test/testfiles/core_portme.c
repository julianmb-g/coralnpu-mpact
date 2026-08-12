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

#include "core_portme.h"

#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined(__riscv) || defined(__riscv__)
int _fstat(int fd, struct stat* st) {
  st->st_mode = S_IFCHR;
  return 0;
}

int _isatty(int fd) { return 1; }

int _close(int fd) { return -1; }

off_t _lseek(int fd, off_t ptr, int dir) {
  (void)fd;
  (void)ptr;
  (void)dir;
  return 0;
}

ssize_t _read(int fd, void* ptr, size_t len) {
  (void)fd;
  (void)ptr;
  (void)len;
  return 0;
}
#endif

volatile int32_t seed1_volatile = 0;
volatile int32_t seed2_volatile = 0;
volatile int32_t seed3_volatile = 0;
volatile int32_t seed4_volatile = ITERATIONS;
volatile int32_t seed5_volatile = 0;

#if defined(__riscv) || defined(__riscv__)
extern volatile uint64_t tohost;
extern volatile uint64_t fromhost;
#else
volatile uint64_t tohost __attribute__((aligned(64))) = 0;
volatile uint64_t fromhost __attribute__((aligned(64))) = 0;
#endif

#if defined(__riscv) || defined(__riscv__)
static void htif_syscall(uint32_t syscall, uintptr_t arg0, uintptr_t arg1,
                         uintptr_t arg2) {
  static volatile uint32_t buf[16] __attribute__((aligned(64))) = {0};
  buf[0] = syscall;
  buf[1] = 0;
  buf[2] = (uint32_t)arg0;
  buf[3] = 0;
  buf[4] = (uint32_t)arg1;
  buf[5] = 0;
  buf[6] = (uint32_t)arg2;
  buf[7] = 0;

  __asm__ volatile("fence rw, rw" ::: "memory");

  uint32_t ptr = (uint32_t)(uintptr_t)buf;
  volatile uint32_t* t = (volatile uint32_t*)&tohost;

  t[0] = ptr;
  t[1] = 0;
  __asm__ volatile("fence rw, rw" ::: "memory");
  t[16] = 1;  // offset 64 (tohost_ready)
  __asm__ volatile("fence rw, rw" ::: "memory");
  t[16] = 0;
  __asm__ volatile("fence rw, rw" ::: "memory");
}

ssize_t _write(int fd, const void* ptr, size_t len) {
  (void)fd;
  htif_syscall(64, 1, (uintptr_t)ptr, len);
  return len;
}
#endif

int ee_printf(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int ret = vfprintf(stdout, fmt, args);
  va_end(args);
  return ret;
}

void abort(void) {
  ee_printf("ABORT CALLED!\n");
  _exit(1);
}

void _exit(int status) {
  // Phase 1 (Buffer Flush): ADR 008
  fflush(stdout);
  fflush(stderr);

  // Phase 2 (Primary Native Halt): ADR 008
  __asm__ volatile(".word 0x08000073\n");

  // Phase 3 (Fallback HTIF Status Write): ADR 008
#if defined(__riscv) || defined(__riscv__)
  tohost = ((uint64_t)status << 1) | 1;
  __asm__ volatile("fence rw, rw" ::: "memory");
#endif

  // Phase 4 (Infinite Loop Guard): ADR 008
  while (1);
}

CORETIMETYPE barebones_clock() {
  uint32_t cycles = 0;
#if defined(__riscv) || defined(__riscv__)
  __asm__ volatile("csrr %0, mcycle" : "=r"(cycles));
#endif
  return (CORETIMETYPE)cycles;
}

CORETIMETYPE get_my_time(void) { return barebones_clock(); }

CORETIMETYPE my_time_diff(CORETIMETYPE final_time, CORETIMETYPE initial_time) {
  return final_time - initial_time;
}

CORETIMETYPE start_time_val = 0;
CORETIMETYPE stop_time_val = 0;

void start_time(void) { start_time_val = get_my_time(); }
void stop_time(void) { stop_time_val = get_my_time(); }
CORE_TICKS get_time(void) {
  return my_time_diff(stop_time_val, start_time_val);
}

secs_ret time_in_secs(CORE_TICKS ticks) {
  float res = (float)ticks / (float)EE_TICK_RESOLUTION;
  return (secs_ret)res;
}

uint32_t default_num_contexts = 1;

// ADR 004: Stateful bump allocator
#define STATIC_MEMBLOCK_SIZE (16 * 1024 * 1024)
static char static_memblock[STATIC_MEMBLOCK_SIZE] __attribute__((aligned(64)));
static size_t current_alloc_offset = 0;

void* portable_malloc(size_t size) {
  // Alignment check
  size_t aligned_size = (size + 63) & ~((size_t)63);

  // Overflow check
  if (aligned_size < size) return NULL;

  // Boundary check
  if (current_alloc_offset + aligned_size > STATIC_MEMBLOCK_SIZE) {
    return NULL;
  }

  void* ptr = (void*)&static_memblock[current_alloc_offset];
  current_alloc_offset += aligned_size;
  return ptr;
}

void portable_free(void* p) { (void)p; }

void reset_portable_malloc() { current_alloc_offset = 0; }

#if defined(__riscv) || defined(__riscv__)
void* _sbrk(ptrdiff_t incr) { return portable_malloc((size_t)incr); }
#endif

void check_dynamic_state(void) {
  uint32_t fcsr_val = 0, fflags_val = 0, frm_val = 0;
  uint32_t vxsat_val = 0, vxrm_val = 0, vlenb_val = 4;
#if defined(__riscv) || defined(__riscv__)
  __asm__ volatile("csrr %0, fcsr" : "=r"(fcsr_val));
  __asm__ volatile("csrr %0, fflags" : "=r"(fflags_val));
  __asm__ volatile("csrr %0, frm" : "=r"(frm_val));
#if defined(__riscv_zve32f)
  __asm__ volatile("csrr %0, vxsat" : "=r"(vxsat_val));
  __asm__ volatile("csrr %0, vxrm" : "=r"(vxrm_val));
  __asm__ volatile("csrr %0, vlenb" : "=r"(vlenb_val));
#endif
#endif
  if (fcsr_val != 0 || fflags_val != 0 || frm_val != 0) {
    ee_printf(
        "ASSERT FAILED: FP CSR state not zero: fcsr: 0x%x, fflags: 0x%x, frm: "
        "0x%x\n",
        fcsr_val, fflags_val, frm_val);
    _exit(1);
  }
#if defined(__riscv_zve32f)
  if (vxsat_val != 0 || vxrm_val != 0 || vlenb_val < 4) {
    ee_printf(
        "ASSERT FAILED: Vector CSR state invalid: vxsat: %d, vxrm: %d, vlenb: "
        "%d\n",
        vxsat_val, vxrm_val, vlenb_val);
    _exit(1);
  }
#endif
}

void portable_init(core_portable* p, int* argc, char* argv[]) {
  (void)argc;
  (void)argv;
  _write(1, "PORTABLE_INIT_START\n", 19);
#if defined(__riscv) || defined(__riscv__)
  setvbuf(stdout, NULL, _IONBF, 0);  // Disable stdout buffering (ADR 024)
#endif
  check_dynamic_state();  // Assert FP CSR state
  p->portable_id = 1;
}

void portable_fini(core_portable* p) {
  p->portable_id = 0;
  _exit(0);
}
