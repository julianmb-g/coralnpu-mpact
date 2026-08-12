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

set -e

if [[ "$#" -ne 3 ]]; then
  printf -- "Usage: %s <newlib_crt0_test.elf> <coralnpu_m3_sim> <newlib_crt0_test.objdump>\n" "$0"
  exit 1
fi

elf_file="$(realpath "$1")"
sim_bin="$(realpath "$2")"
objdump_file="$(realpath "$3")"

if [[ ! -f "$elf_file" ]] || [[ ! -f "$sim_bin" ]]; then
  printf -- "Error: ELF file or simulator binary not found.\n"
  exit 1
fi

# Run the ELF in the simulator directly with a short timeout to catch the expected hang
# Stock newlib crt0 immediately traps/hangs on this platform.
# We wrap the command in set +e and set -e to handle the non-zero exit code (124) from timeout safely.
set +e
output="$(timeout 5s "$sim_bin" --semihost_htif --allow_memory_region=0x1000:0x4000000:rx --allow_memory_region=0x4001000:0x4000000:rw "$elf_file" 2>&1)"
exit_code=$?
set -e

printf -- "--- SIMULATOR OUTPUT ---\n"
printf "%s\n" "$output"
printf -- "--- END SIMULATOR OUTPUT ---\n"

if [[ $exit_code -eq 0 ]] && printf "%s" "$output" | grep -q "Correct operation validated"; then
  printf -- "Security/Compatibility Violation: Stock newlib crt0 unexpectedly succeeded.\n"
  exit 1
fi
if [[ $exit_code -eq 124 ]]; then
  printf -- "newlib_crt0_test PASSED: Stock newlib hung (Simulation timeout), correctly identifying incompatibility.\n"
  exit 0
elif [[ -z "$output" ]]; then
  printf -- "newlib_crt0_test FAILED: Stock newlib produced no output, expected a deterministic trap.\n"
  exit 1
elif [[ $exit_code -eq 0 ]] && ! printf "%s" "$output" | grep -q "Correct operation validated"; then
  printf -- "newlib_crt0_test PASSED: Stock newlib encountered a fault and gracefully exited via _exit() before validating operation.\n"
  exit 0
else
  printf -- "newlib_crt0_test FAILED: Stock newlib failed but not with an expected trap (InstructionAccessFault, IllegalInstruction, or MemoryAccessFault).\n"
  exit 1
fi
