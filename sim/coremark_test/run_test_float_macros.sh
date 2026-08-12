#!/bin/bash
# Copyright 2026 Google LLC
# Licensed under the Apache License, Version 2.0 (the "License");

set -euo pipefail

elf_path=$(find . -name test_float_macros.elf -print -quit)

if [[ ! -f "${elf_path}" ]]; then
    echo "Error: ELF file not found at ${elf_path}"
    exit 1
fi

echo "Running test_float_macros on simulator..."
# Run the simulator on the compiled ELF
"sim/coralnpu_m3_sim" --semihost_htif --allow_memory_region=0x1000:0x4000000:rx --allow_memory_region=0x4001000:0x4000000:rw "${elf_path}" > "${TEST_TMPDIR}/sim_output.log" 2>&1
sim_exit_code=$?

cat "${TEST_TMPDIR}/sim_output.log"

if [[ ${sim_exit_code} -ne 0 ]]; then
    echo "Error: Simulator exited with code ${sim_exit_code}"
    exit 1
fi

if ! grep -q "test_float_macros PASSED" "${TEST_TMPDIR}/sim_output.log"; then
    echo "Error: Validation output not found."
    exit 1
fi

echo "test_float_macros completed successfully."
exit 0
