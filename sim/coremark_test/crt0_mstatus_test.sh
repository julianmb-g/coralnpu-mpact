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

set -ex

if [[ "$#" -ne 2 ]]; then
  echo "Usage: $0 <coralnpu_m3_sim> <crt0_mstatus_test.elf>"
  exit 1
fi

sim_bin="$(realpath "$1")"
elf_file="$(realpath "$2")"

# Run the ELF in the simulator directly with correct memory mapping
output="$(timeout 60s "$sim_bin" --semihost_htif --allow_memory_region=0x1000:0x4000000:rx --allow_memory_region=0x4001000:0x4000000:rw "$elf_file" 2>&1)"
exit_code=$?

echo "--- SIMULATOR OUTPUT ---"
echo "$output"
echo "--- END SIMULATOR OUTPUT ---"

# Finding #331: Support both 0x00004000 and 0x00004400 since mstatus.VS is read-only in some simulator versions.
if [[ $exit_code -eq 0 ]] && echo "$output" | grep -E -q "mstatus: 0x00004(0|4)00" && echo "$output" | grep -q "mpause instruction received"; then
  echo "mstatus initialized correctly"
  exit 0
else
  echo "mstatus initialization failed or simulator crashed: exit_code=$exit_code"
  exit 1
fi
