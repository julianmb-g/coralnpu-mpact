#include "core_portme.h"

int main() {
  ee_printf("Hello World!\n");
  ee_printf("Int: %d\n", 123);
  ee_printf("Hex: %x\n", 123);
  ee_printf("Float: %f\n", 3.14159);
  ee_printf("String: %s\n", "Test String");
  ee_printf("Char: %c\n", 'X');
  ee_printf("Percent: %%\n");
  ee_printf("Done!\n");
  return 0;
}
