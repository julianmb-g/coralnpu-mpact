#!/bin/bash
# Test case for matrix_mul_vect to ensure vector instruction coverage (Finding #226).

set -e

if [[ "$#" -ne 1 ]]; then
  printf "Usage: %s <coremark_unified.objdump>\n" "$0"
  exit 1
fi

objdump_file=$(realpath "$1")

printf "DEBUG: objdump_file=%s\n" "$objdump_file"

# Define the regex for vector floating-point instructions.
vector_fp_regex="[[:space:]](vfadd\.|vfsub\.|vfmul\.|vfmacc\.|vfdiv\.|vfmadd\.|vfmsub\.|vfnmacc\.|vfmsac\.|vfnmsac\.|vfnmadd\.|vfnmsub\.|vfcvt\.|vle32\.|vse32\.|vfmv\.)"

# Check for vector instructions in matrix_add_const.
printf "DEBUG: Checking for vector instructions in matrix_add_const...\n"
if ! awk "/<matrix_add_const>:/ {p=1; next} /^[0-9a-fA-F]+ <.*>:/ {p=0} p" "$objdump_file" | grep -E "$vector_fp_regex" > /dev/null; then
  printf "Vector Instruction Check (matrix_add_const): FAILED - No vector floating-point instructions found.\n"
  exit 1
fi
printf "Vector Instruction Check (matrix_add_const): PASSED\n"

printf "test_matrix.sh: PASSED\n"
exit 0
