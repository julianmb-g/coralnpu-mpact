#include <limits.h>

#include "core_portme.h"

int main() {
  ee_printf("Hello World!\n");
  ee_printf("Int: %d\n", 123);
  ee_printf("Hex: %x\n", 123);
  ee_printf("Float: %f\n", 3.14159f);
  ee_printf("String: %s\n", "Test String");
  ee_printf("Char: %c\n", 'X');
  ee_printf("Percent: %%\n");
  ee_printf("Large Hex 1: %x\n", 0xFFFFFFFF);
  ee_printf("Large Hex 2: %x\n", 0x80000000);
  ee_printf("INT_MAX: %d\n", INT_MAX);
  ee_printf("INT_MIN: %d\n", INT_MIN);
  ee_printf("UINT_MAX: %u\n", UINT_MAX);
  ee_printf("Large Float 1: %f\n", 1.23456789e+38f);
  ee_printf("Large Float 2: %f\n", 1.23456789e-38f);
  ee_printf("Done!\n");
  return 0;
}
