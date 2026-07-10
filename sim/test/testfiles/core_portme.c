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

volatile ee_s32 seed1_volatile = 0;
volatile ee_s32 seed2_volatile = 0;
volatile ee_s32 seed3_volatile = 0;
volatile ee_s32 seed4_volatile = ITERATIONS;
volatile ee_s32 seed5_volatile = 0;
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
  const char* cptr = (const char*)ptr;
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

  uint32_t ptr = (uint32_t)(uintptr_t)buf;
  volatile uint32_t* t = (volatile uint32_t*)&tohost;
  volatile uint32_t* tr = (volatile uint32_t*)&tohost_ready;

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
    write(arg0, (const void*)arg1, (size_t)arg2);
  }
#endif
}

static void uart_send_char(char c) { htif_syscall(64, 1, (uintptr_t)&c, 1); }

typedef struct {
  int decpt;
  int sign;
} cvt_result_t;
static cvt_result_t ecvtbuf(double arg, int ndigits, char* buf);

void ee_printf(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int ret = 0;
  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      int pad = 0;
      if (*fmt == '0') {
        fmt++;
        if (*fmt >= '0' && *fmt <= '9') {
          pad = *fmt - '0';
          fmt++;
        }
      }
      int is_long = 0;
      if (*fmt == 'l') {
        is_long = 1;
        fmt++;
      }
      if (*fmt == 'u') {
        unsigned long val =
            is_long ? va_arg(args, unsigned long) : va_arg(args, unsigned int);
        char buf[32];
        int i = 0;
        unsigned long uval = val;
        if (uval == 0) {
          buf[i++] = '0';
        } else {
          while (uval > 0) {
            buf[i++] = '0' + (uval % 10);
            uval /= 10;
          }
        }
        while (i < pad) {
          buf[i++] = '0';
        }
        while (i > 0) {
          uart_send_char(buf[--i]);
          ret++;
        }
      } else if (*fmt == 'd') {
        long val = is_long ? va_arg(args, long) : va_arg(args, int);
        char buf[32];
        int i = 0;
        unsigned long uval;
        if (val < 0) {
          uart_send_char('-');
          ret++;
          uval = (unsigned long)-(val + 1) + 1;
        } else {
          uval = (unsigned long)val;
        }
        if (uval == 0) {
          buf[i++] = '0';
        } else {
          while (uval > 0) {
            buf[i++] = '0' + (uval % 10);
            uval /= 10;
          }
        }
        while (i < pad) {
          buf[i++] = '0';
        }
        while (i > 0) {
          uart_send_char(buf[--i]);
          ret++;
        }
      } else if (*fmt == 'x' || *fmt == 'X') {
        unsigned long val =
            is_long ? va_arg(args, unsigned long) : va_arg(args, unsigned int);
        char buf[32];
        int i = 0;
        int is_upper = (*fmt == 'X');
        unsigned long uval = val;
        if (uval == 0) {
          buf[i++] = '0';
        } else {
          while (uval > 0) {
            int rem = uval % 16;
            if (is_upper) {
              buf[i++] = (rem < 10) ? '0' + rem : 'A' + rem - 10;
            } else {
              buf[i++] = (rem < 10) ? '0' + rem : 'a' + rem - 10;
            }
            uval /= 16;
          }
        }
        while (i < pad) {
          buf[i++] = '0';
        }
        while (i > 0) {
          uart_send_char(buf[--i]);
          ret++;
        }
      } else if (*fmt == 's') {
        char* s = va_arg(args, char*);
        while (s && *s) {
          uart_send_char(*s++);
          ret++;
        }
      } else if (*fmt == 'f') {
        double val = va_arg(args, double);
        char buf[64];
        cvt_result_t res = ecvtbuf(val, 6, buf);
        if (res.sign) {
          uart_send_char('-');
          ret++;
        }
        int i = 0;
        while (buf[i]) {
          if (i == res.decpt) {
            uart_send_char('.');
            ret++;
          }
          uart_send_char(buf[i++]);
          ret++;
        }
      } else if (*fmt == 'c') {
        int c = va_arg(args, int);
        uart_send_char((char)c);
        ret++;
      } else if (*fmt == '%') {
        uart_send_char('%');
        ret++;
      }
    } else {
      uart_send_char(*fmt);
      ret++;
    }
    fmt++;
  }
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
  uint64_t payload = ((uint64_t)status << 1) | 1ULL;
  volatile uint32_t* t = (volatile uint32_t*)&tohost;
  t[0] = (uint32_t)payload;
  t[1] = (uint32_t)(payload >> 32);
  __asm__ volatile("fence rw, rw" ::: "memory");
  volatile uint32_t* tr = (volatile uint32_t*)&tohost_ready;
  tr[0] = 1;
  while (1);
}
#endif
static CORETIMETYPE barebones_clock() {
  uint32_t cycles;
#if defined(__riscv) || defined(__riscv__)
  __asm__ volatile("csrr %0, mcycle" : "=r"(cycles));
#else
  cycles = 0;
#endif
  return (CORETIMETYPE)cycles;
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

static const ee_f32 ee_ticks_per_sec_float = (ee_f32)EE_TICK_RESOLUTION;
secs_ret time_in_secs(CORE_TICKS ticks) {
  float t = (float)ticks;
  float res = t / ee_ticks_per_sec_float;
  return (secs_ret)res;
}

static cvt_result_t fcvtbuf(double arg, int ndigits, char* buf) {
  cvt_result_t res = {0, 0};
  if (isnan(arg)) {
    res.sign = 0;
    res.decpt = 4;  // Beyond string length to avoid dot
    buf[0] = 'n';
    buf[1] = 'a';
    buf[2] = 'n';
    buf[3] = '\0';
    return res;
  }
  if (isinf(arg)) {
    res.sign = (arg < 0);
    res.decpt = 4;  // Beyond string length to avoid dot
    buf[0] = 'i';
    buf[1] = 'n';
    buf[2] = 'f';
    buf[3] = '\0';
    return res;
  }

  res.sign = (arg < 0);
  if (arg < 0) arg = -arg;

  // Rounding: add 0.5 * 10^(-ndigits)
  double round_factor = 0.5;
  for (int j = 0; j < ndigits; j++) {
    round_factor /= 10.0;
  }
  arg += round_factor;

  if (arg > (double)INT_MAX) {
    res.decpt = 4;  // Beyond string length to avoid dot
    buf[0] = 'o';
    buf[1] = 'v';
    buf[2] = 'f';
    buf[3] = '\0';
    return res;
  }

  int int_part = (int)arg;
  double frac_part = arg - int_part;

  int i = 0;
  if (int_part == 0) {
    buf[i++] = '0';
    res.decpt = 0;
  } else {
    int temp = int_part;
    int p = 0;
    while (temp > 0) {
      p++;
      temp /= 10;
    }
    res.decpt = p;
    temp = int_part;
    for (int j = p - 1; j >= 0; j--) {
      buf[j] = '0' + (temp % 10);
      temp /= 10;
    }
    i = p;
  }

  for (int j = 0; j < ndigits; j++) {
    frac_part *= 10.0;
    int d = (int)frac_part;
    buf[i++] = '0' + d;
    frac_part -= d;
  }
  buf[i] = '\0';

  if (res.decpt == 0) res.decpt = 1;  // Standard fcvt behavior for 0
  return res;
}

static cvt_result_t ecvtbuf(double arg, int ndigits, char* buf) {
  return fcvtbuf(arg, ndigits, buf);
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
  uint32_t aligned_size = (size + 63) & ~63;  // Align size to 64 bytes
  if (current_alloc_offset + aligned_size > sizeof(static_memblock)) {
#ifdef __cplusplus
    return nullptr;
#else
    return NULL;
#endif
  }
  void* ptr = &static_memblock[current_alloc_offset];
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
    return (void*)-1;
  }
  heap_end += incr;
  return (void*)prev_heap_end;
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
