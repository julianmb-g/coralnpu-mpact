#include <stdio.h>
#include <string.h>

#include "core_portme.h"

// Helper to calculate aligned size (64 bytes)
static uint32_t get_aligned_size(uint32_t size) { return (size + 63) & ~63; }

int main() {
  reset_portable_malloc();

  void* ptr1 = portable_malloc(100);
  if (ptr1 == NULL) {
    ee_printf("portable_malloc(100) failed\n");
    return 1;
  }
  ee_printf("ptr1: 0x%x\n", (uintptr_t)ptr1);

  void* ptr2 = portable_malloc(200);
  if (ptr2 == NULL) {
    ee_printf("portable_malloc(200) failed\n");
    return 1;
  }
  ee_printf("ptr2: 0x%x\n", (uintptr_t)ptr2);

  void* ptr3 = portable_malloc(50);
  if (ptr3 == NULL) {
    ee_printf("portable_malloc(50) failed\n");
    return 1;
  }
  ee_printf("ptr3: 0x%x\n", (uintptr_t)ptr3);

  if (ptr1 == ptr2 || ptr1 == ptr3 || ptr2 == ptr3) {
    ee_printf("Error: Some pointers are the same!\n");
    return 1;
  }

  uintptr_t uptr1 = (uintptr_t)ptr1;
  uintptr_t uptr2 = (uintptr_t)ptr2;
  uintptr_t uptr3 = (uintptr_t)ptr3;

  uint32_t aligned_size1 = get_aligned_size(100);
  uint32_t aligned_size2 = get_aligned_size(200);

  // Check non-overlapping: ptr2 must start at or after ptr1 + aligned_size1
  if (uptr2 < uptr1 + aligned_size1) {
    ee_printf(
        "Error: ptr2 overlaps with ptr1! uptr2: 0x%lx, expected >= 0x%lx\n",
        uptr2, uptr1 + aligned_size1);
    return 1;
  }

  // Check non-overlapping: ptr3 must start at or after ptr2 + aligned_size2
  if (uptr3 < uptr2 + aligned_size2) {
    ee_printf(
        "Error: ptr3 overlaps with ptr2! uptr3: 0x%lx, expected >= 0x%lx\n",
        uptr3, uptr2 + aligned_size2);
    return 1;
  }

  // Test allocation limit
  reset_portable_malloc();
  void* ptr_large = portable_malloc(2000001);
  if (ptr_large != NULL) {
    ee_printf("Error: Allocation larger than block size did not fail!\n");
    return 1;
  }

  reset_portable_malloc();
  void* ptr_half1 = portable_malloc(1000000);
  if (ptr_half1 == NULL) return 1;
  void* ptr_half2 = portable_malloc(1000000);
  if (ptr_half2 == NULL) return 1;
  void* ptr_extra = portable_malloc(1);
  if (ptr_extra != NULL) {
    ee_printf("Error: Allocation after memory full did not fail!\n");
    return 1;
  }

  ee_printf("portable_malloc tests passed\n");
  return 0;
}
