#!/bin/bash
# Copyright 2026 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# run_final_test.sh

set -e

# Paths relative to current directory in test execution
if [ -z "$1" ]; then
    echo "Usage: $0 <simulator_executable>"
    exit 1
fi
sim_path="$1"
if [ ! -x "$sim_path" ]; then
    echo "Simulator executable not found or not executable: $sim_path"
    exit 1
fi
elf_path="sim/coremark_test/coremark_unified.elf"
objdump_path="sim/coremark_test/coremark_unified.objdump"
common_cflags_sh="sim/coremark_test/common_cflags.sh"
validate_sh="sim/coremark_test/validate_output.sh"

# 4.1.10: Measure execution duration (ADR 013)
start_time=$(date +%s)
echo "Starting run_final_test.sh"
# Run the simulator with semihosting and appropriate memory regions
# Stream output directly to stdout to ensure visibility on timeout
output_file=$(mktemp)
"${sim_path}" --semihost_htif --exit_on_ebreak --allow_memory_region=0x1000:0x4000000:rx --allow_memory_region=0x4001000:0x4000000:rw "${elf_path}" 2>&1 | tee "$output_file"
end_time=$(date +%s)
exit_code=${PIPESTATUS[0]}
output=$(cat "$output_file")
rm "$output_file"
duration=$((end_time - start_time))

echo "--- SIMULATOR COMPLETED WITH EXIT CODE ${exit_code} ---"
echo "--- RAW SIMULATOR OUTPUT ---"
echo "$output"
echo "----------------------------"

# Source the validation logic
source "${validate_sh}"

# Perform rigorous validation
validate_coremark_output "${output}" "${objdump_path}" "${common_cflags_sh}" "${exit_code}"
