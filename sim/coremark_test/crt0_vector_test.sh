#!/bin/bash
# Pre-flight check for vector initialization in coremark_unified.objdump

# Locate coremark_unified.objdump using robust relative paths and Bazel environment variables
objdump_path=""
for path in \
  "${TEST_SRCDIR}/google3/learning/brain/research/kelvin/sim/coremark_test/coremark_unified.objdump" \
  "learning/brain/research/kelvin/sim/coremark_test/coremark_unified.objdump" \
  "$(dirname "$0")/coremark_unified.objdump"; do
  if [[ -f "$path" ]]; then
    objdump_path="$path"
    break
  fi
done

if [[ -z "$objdump_path" ]]; then
  printf "Error: coremark_unified.objdump not found.\n"
  exit 1
fi

# Extract _start section to scope the test and prevent false positives from benchmark code
start_section=$(awk '/<_start>:/ {p=1; next} /^[0-9a-f]+ <.*>:/ {p=0} p {print}' "$objdump_path")

if [[ -z "$start_section" ]]; then
  printf "Error: _start function not found in objdump.\n"
  exit 1
fi

# Semantic constants for expected mstatus initialization (0x6600 for mstatus.VS and mstatus.FS)
expected_mstatus_lui="0x6"     # Upper 20 bits (0x6000)
expected_mstatus_addi="1536"   # Lower 12 bits (0x600)

# Check if vector unit is enabled (mstatus.VS initialization)
# In objdump, 'li t0, 0x00006600' expands to 'lui t0, 0x6' and 'addi t0, t0, 1536'
if ! printf "%b" "$start_section" | grep -qE "lui[[:space:]]+t0,${expected_mstatus_lui}" || ! printf "%b" "$start_section" | grep -qE "addi[[:space:]]+t0,t0,${expected_mstatus_addi}"; then
  printf "coremark_unified.objdump does not correctly initialize mstatus.VS in _start (missing lui/addi for 0x6600)\n"
  exit 1
fi

if ! printf "%b" "$start_section" | grep -qE "csrs[[:space:]]+mstatus,t0"; then
  printf "coremark_unified.objdump does not correctly initialize mstatus.VS in _start (missing csrs)\n"
  exit 1
fi

# Check vector register zeroing
for reg in 0 4 8 12 16 20 24 28; do
  if ! printf "%b" "$start_section" | grep -qE "vmv\.v\.i[[:space:]]+v${reg},0"; then
    printf "coremark_unified.objdump does not correctly zero-initialize vector register v${reg} in _start\n"
    exit 1
  fi
done

printf "Vector unit initialization check: PASSED\n"
exit 0
