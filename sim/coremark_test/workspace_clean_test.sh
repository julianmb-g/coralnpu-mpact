#!/bin/bash
# Copyright 2026 Google LLC
set -e

# Locate target directory
TARGET_DIR=""
# Try to follow symlinks to find original workspace
REAL_PATH=$(readlink -f "$0" 2>/dev/null || echo "$0")
REAL_DIR=$(dirname "$REAL_PATH")

# If running inside a Blaze snapshot, try to resolve to active workspace
if [[ "$REAL_DIR" =~ ^(.*)/\.snapshot/[0-9]+/(.*)$ ]]; then
  REAL_DIR="${BASH_REMATCH[1]}/${BASH_REMATCH[2]}"
fi

for path in "$REAL_DIR" "${TEST_SRCDIR}/google3/learning/brain/research/kelvin/sim/coremark_test" "learning/brain/research/kelvin/sim/coremark_test" "$(dirname "$0")"; do
  if [ -d "$path" ] && [ -f "$path/BUILD" ]; then
    TARGET_DIR="$path"
    break
  fi
done

if [ -z "$TARGET_DIR" ]; then
  echo "Error: Target directory not found."
  exit 1
fi

echo "Checking for unexpected files in $TARGET_DIR..."

# Whitelist of allowed files
ALLOWED_FILES="(BUILD|coremark_builder.Dockerfile|build_unified_asm.sh|build_unified_elf.sh|run_validation.sh|diff_test.sh|validate_formatting.sh|crt0_vector_test.sh|run_validation_dynamic.sh|newlib_crt0_test.sh|workspace_clean_test.sh|test_ee_printf.sh|test_ee_printf.c|portable_malloc_test.sh|portable_malloc_test.c|coremark-builder.tar|coremark_unified.S|coremark_unified.elf|coremark_unified.map|coremark_unified.objdump|coremark_individual.elf|coremark_individual.map|coremark_individual.objdump|core_portme.S|core_list_join.S|core_state.S|core_main.S|core_util.S|core_matrix.S|build_indiv_asm.sh|update.sh)"

# Find all files in TARGET_DIR and filter out the allowed ones
UNEXPECTED_FILES=$(find "$TARGET_DIR" -maxdepth 1 -type f -printf "%P\n" | grep -vE "^$ALLOWED_FILES\$" || true)

if [ -n "$UNEXPECTED_FILES" ]; then
  echo "Error: Found unexpected files in $TARGET_DIR:"
  echo "$UNEXPECTED_FILES"
  exit 1
fi

echo "Workspace clean validation passed."
exit 0
