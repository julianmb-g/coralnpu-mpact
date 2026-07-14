#!/bin/bash
set -e

workspace_dir="$(blaze info workspace)"
cd "$workspace_dir"

printf "Building coremark_unified_elf...\n"
blaze build //learning/brain/research/kelvin/sim/coremark_test:coremark_unified_elf

genfiles_dir="$(blaze info blaze-genfiles)"

printf "Copying generated assembly to sim/test/testfiles/coremark_unified.S...\n"
cp -f "$genfiles_dir/learning/brain/research/kelvin/sim/coremark_test/coremark_unified.S" \
  learning/brain/research/kelvin/sim/test/testfiles/coremark_unified.S

printf "Copying generated map to sim/test/testfiles/coremark_unified.map...\n"
cp -f "$genfiles_dir/learning/brain/research/kelvin/sim/coremark_test/coremark_unified.map" \
  learning/brain/research/kelvin/sim/test/testfiles/coremark_unified.map

printf "Done.\n"
