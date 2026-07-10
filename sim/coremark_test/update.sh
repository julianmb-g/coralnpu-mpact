#!/bin/bash
set -e

WORKSPACE_DIR="$(blaze info workspace)"
cd "$WORKSPACE_DIR"

echo "Building coremark_unified_elf..."
blaze build //learning/brain/research/kelvin/sim/coremark_test:coremark_unified_elf

GENFILES_DIR="$(blaze info blaze-genfiles)"

echo "Copying generated assembly to sim/test/testfiles/coremark_unified.S..."
cp -f "$GENFILES_DIR/learning/brain/research/kelvin/sim/coremark_test/coremark_unified.S" \
  learning/brain/research/kelvin/sim/test/testfiles/coremark_unified.S

echo "Copying generated map to sim/test/testfiles/coremark_unified.map..."
cp -f "$GENFILES_DIR/learning/brain/research/kelvin/sim/coremark_test/coremark_unified.map" \
  learning/brain/research/kelvin/sim/test/testfiles/coremark_unified.map

echo "Done."
