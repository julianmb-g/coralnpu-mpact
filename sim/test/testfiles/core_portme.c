#include "core_portme.h"

#ifdef __cplusplus
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#else
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#endif
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>

#include "coremark_authentic.h"

static void uart_send_char(char c);

#if defined(__riscv) || defined(__riscv__)
int _fstat(int fd, struct stat* st) {
  st->st_mode = S_IFCHR;
  return 0;
}

int _isatty(int fd) { return 1; }

int _close(int fd) { return -1; }

off_t _lseek(int fd, off_t ptr, int dir) { return 0; }

ssize_t _read(int fd, void* ptr, size_t len) { return 0; }
#endif

volatile int32_t seed1_volatile = 0;
volatile int32_t seed2_volatile = 0;
volatile int32_t seed3_volatile = 0;
volatile int32_t seed4_volatile = ITERATIONS;
volatile int32_t seed5_volatile = 0;
#if defined(__riscv) || defined(__riscv__)
volatile uint64_t tohost_ready
    __attribute__((section(".tohost"), aligned(64))) = 0;
volatile uint64_t tohost __attribute__((section(".tohost"), aligned(64))) = 0;
volatile uint64_t fromhost_ready
    __attribute__((section(".tohost"), aligned(64))) = 0;
volatile uint64_t fromhost __attribute__((section(".tohost"), aligned(64))) = 0;
#else
volatile uint64_t tohost_ready __attribute__((aligned(64))) = 0;
volatile uint64_t tohost __attribute__((aligned(64))) = 0;
volatile uint64_t fromhost_ready __attribute__((aligned(64))) = 0;
volatile uint64_t fromhost __attribute__((aligned(64))) = 0;
#endif

int main(void);

#if defined(__riscv) || defined(__riscv__)
ssize_t _write(int fd, const void* ptr, size_t len) {
  (void)fd;
#ifdef __cplusplus
  const char* cptr = reinterpret_cast<const char*>(ptr);
#else
  const char* cptr = (const char*)ptr;
#endif
  for (size_t i = 0; i < len; i++) {
    uart_send_char(cptr[i]);
  }
  return len;
}
#endif

static void htif_syscall(uint32_t syscall, uintptr_t arg0, uintptr_t arg1,
                         uintptr_t arg2) {
#if defined(__riscv) || defined(__riscv__)
  volatile uint32_t buf[16] __attribute__((aligned(64))) = {0};
  buf[0] = syscall;
  buf[1] = 0;
  buf[2] = arg0;
  buf[3] = 0;
  buf[4] = arg1;
  buf[5] = 0;
  buf[6] = arg2;
  buf[7] = 0;

  __asm__ volatile("fence rw, rw" ::: "memory");

#ifdef __cplusplus
  uint32_t ptr = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(buf));
  volatile uint32_t* t = reinterpret_cast<volatile uint32_t*>(&tohost);
  volatile uint32_t* tr = reinterpret_cast<volatile uint32_t*>(&tohost_ready);
#else
  uint32_t ptr = (uint32_t)(uintptr_t)buf;
  volatile uint32_t* t = (volatile uint32_t*)&tohost;
  volatile uint32_t* tr = (volatile uint32_t*)&tohost_ready;
#endif

  t[0] = ptr;
  __asm__ volatile("fence rw, rw" ::: "memory");
  tr[0] = 1;
  __asm__ volatile("fence rw, rw" ::: "memory");

  while (fromhost_ready == 0) {
    __asm__ volatile("fence rw, rw" ::: "memory");
  }
  fromhost_ready = 0;
  tr[0] = 0;
#else
  if (syscall == 64) {
#ifdef __cplusplus
    write(arg0, reinterpret_cast<const void*>(arg1), static_cast<size_t>(arg2));
#else
    write(arg0, (const void*)arg1, (size_t)arg2);
#endif
  }
#endif
}

static void uart_send_char(char c) { htif_syscall(64, 1, (uintptr_t)&c, 1); }

void ee_printf(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
#if defined(__riscv) || defined(__riscv__)
  vfprintf(stdout, fmt, args);
#else
  vprintf(fmt, args);
#endif
  va_end(args);
}

#if defined(__riscv) || defined(__riscv__)
void _exit(int status);
#endif

void abort(void) {
  ee_printf("ABORT CALLED!\n");
  _exit(1);
}

#if defined(__riscv) || defined(__riscv__)
void _exit(int status) {
  // Enforce mpause mandate for all exit paths
  __asm__ volatile(".word 0x08000073\n");

  // Fallback to HTIF to signal exit status
#ifdef __cplusplus
  uint64_t payload = (static_cast<uint64_t>(status) << 1) | 1ULL;
  volatile uint32_t* t = reinterpret_cast<volatile uint32_t*>(&tohost);
  t[0] = static_cast<uint32_t>(payload);
  t[1] = static_cast<uint32_t>(payload >> 32);
  __asm__ volatile("fence rw, rw" ::: "memory");
  volatile uint32_t* tr = reinterpret_cast<volatile uint32_t*>(&tohost_ready);
#else
  uint64_t payload = ((uint64_t)status << 1) | 1ULL;
  volatile uint32_t* t = (volatile uint32_t*)&tohost;
  t[0] = (uint32_t)payload;
  t[1] = (uint32_t)(payload >> 32);
  __asm__ volatile("fence rw, rw" ::: "memory");
  volatile uint32_t* tr = (volatile uint32_t*)&tohost_ready;
#endif
  tr[0] = 1;
  while (1);
}
#endif
static CORETIMETYPE barebones_clock() {
  uint32_t cycles = 0;
#if defined(__riscv) || defined(__riscv__)
  __asm__ volatile("csrr %0, mcycle" : "=r"(cycles));
#endif
#ifdef __cplusplus
  return static_cast<CORETIMETYPE>(cycles);
#else
  return (CORETIMETYPE)cycles;
#endif
}

static inline CORETIMETYPE get_my_time(void) { return barebones_clock(); }

static inline CORETIMETYPE my_time_diff(CORETIMETYPE final_time,
                                        CORETIMETYPE initial_time) {
  return final_time - initial_time;
}

#define TIMER_RES_DIVIDER 1

static CORETIMETYPE start_time_val, stop_time_val;

void start_time(void) { start_time_val = get_my_time(); }
void stop_time(void) { stop_time_val = get_my_time(); }
CORE_TICKS get_time(void) {
  return my_time_diff(stop_time_val, start_time_val);
}

#ifdef __cplusplus
static const float ee_ticks_per_sec_float =
    static_cast<float>(EE_TICK_RESOLUTION);
#else
static const float ee_ticks_per_sec_float = (float)EE_TICK_RESOLUTION;
#endif
secs_ret time_in_secs(CORE_TICKS ticks) {
#ifdef __cplusplus
  float t = static_cast<float>(ticks);
#else
  float t = (float)ticks;
#endif
  float res = t / ee_ticks_per_sec_float;
#ifdef __cplusplus
  return static_cast<secs_ret>(res);
#else
  return (secs_ret)res;
#endif
}

/*
 * Upstream CoreMark Configuration:
 * default_num_contexts specifies the context count for standard benchmark runs.
 * Declared as extern uint32_t to match core_main.c's C-linkage reference.
 * Strictly read-only at runtime for single-context simulations.
 */
uint32_t default_num_contexts = 1;

/*
 * Static Memory Buffer (Audited & Isolated):
 * Allocates 2MB of raw memory static block.
 * - Uses 'char' instead of 'uint8_t' to respect strict aliasing rules
 * (compliance with "uint8_t is not byte").
 * - Declared as 'static' to restrict its linkage scope strictly to this
 * translation unit.
 * - Aligned to 64 bytes to guarantee cache alignment on target Hardware
 * (CoralNPU M3).
 */
static char static_memblock[2000000] __attribute__((aligned(64)));
static uint32_t current_alloc_offset = 0;

void* portable_malloc(uint32_t size) {
  // Finding #268: Add explicit pre-alignment bounds check to prevent integer
  // overflow.
  if (size > UINT_MAX - 63) {
#ifdef __cplusplus
    return nullptr;
#else
    return NULL;
#endif
  }

  uint32_t aligned_size = (size + 63) & ~63;  // Align size to 64 bytes
  if (current_alloc_offset + aligned_size > sizeof(static_memblock)) {
#ifdef __cplusplus
    return nullptr;
#else
    return NULL;
#endif
  }
#ifdef __cplusplus
  void* ptr = reinterpret_cast<void*>(&static_memblock[current_alloc_offset]);
#else
  void* ptr = (void*)&static_memblock[current_alloc_offset];
#endif
  current_alloc_offset += aligned_size;
  return ptr;
}

void portable_free(void* p) {
  (void)p; /* Simple bump allocator doesn't free */
}

// Function to reset the allocator, useful for testing
void reset_portable_malloc() { current_alloc_offset = 0; }

#if defined(__riscv) || defined(__riscv__)
void* _sbrk(ptrdiff_t incr) {
  extern char __heap_start;
  extern char __heap_end;
  static char* heap_end;
  char* prev_heap_end = 0;

  if (heap_end == 0) {
    heap_end = &__heap_start;
  }
  prev_heap_end = heap_end;
  if (heap_end + incr > &__heap_end || heap_end + incr < &__heap_start) {
#ifdef __cplusplus
    return reinterpret_cast<void*>(-1);
#else
    return (void*)-1;
#endif
  }
  heap_end += incr;
#ifdef __cplusplus
  return reinterpret_cast<void*>(prev_heap_end);
#else
  return (void*)prev_heap_end;
#endif
}
#endif

void portable_init(core_portable* p, int* argc, char* argv[]) {
  (void)argc;
  (void)argv;
  p->portable_id = 1;
}

void portable_fini(core_portable* p) {
  p->portable_id = 0;
  _exit(0);
}
