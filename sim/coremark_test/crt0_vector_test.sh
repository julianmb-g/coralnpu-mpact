#!/bin/bash
# Pre-flight check for vector initialization in coremark_unified.objdump

# Locate coremark_unified.objdump using robust relative paths and Bazel environment variables
OBJDUMP_PATH=""
for path in \
  "${TEST_SRCDIR}/google3/learning/brain/research/kelvin/sim/coremark_test/coremark_unified.objdump" \
  "learning/brain/research/kelvin/sim/coremark_test/coremark_unified.objdump" \
  "$(dirname "$0")/coremark_unified.objdump"; do
  if [ -f "$path" ]; then
    OBJDUMP_PATH="$path"
    break
  fi
done

if [ -z "$OBJDUMP_PATH" ]; then
  echo "Error: coremark_unified.objdump not found."
  exit 1
fi

# Extract _start section to scope the test and prevent false positives from benchmark code
START_SECTION=$(awk '/<_start>:/ {p=1; next} /^[0-9a-f]+ <.*>:/ {p=0} p {print}' "$OBJDUMP_PATH")

if [ -z "$START_SECTION" ]; then
  echo "Error: _start function not found in objdump."
  exit 1
fi

# Check if vector unit is enabled (mstatus.VS initialization)
# In objdump, 'li t0, 0x00006600' expands to 'lui t0, 0x6' and 'addi t0, t0, 1536'
if ! echo "$START_SECTION" | grep -qE "lui[[:space:]]+t0,0x6" || ! echo "$START_SECTION" | grep -qE "addi[[:space:]]+t0,t0,1536"; then
  echo "coremark_unified.objdump does not correctly initialize mstatus.VS in _start (missing lui/addi for 0x6600)"
  exit 1
fi

if ! echo "$START_SECTION" | grep -qE "csrs[[:space:]]+mstatus,t0"; then
  echo "coremark_unified.objdump does not correctly initialize mstatus.VS in _start (missing csrs)"
  exit 1
fi

# Check vector register zeroing
for reg in 0 4 8 12 16 20 24 28; do
  if ! echo "$START_SECTION" | grep -qE "vmv\.v\.i[[:space:]]+v${reg},0"; then
    echo "coremark_unified.objdump does not correctly zero-initialize vector register v${reg} in _start"
    exit 1
  fi
done

echo "Vector unit initialization check: PASSED"
exit 0
