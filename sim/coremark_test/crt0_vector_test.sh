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

if [[ "$#" -ne 2 ]]; then
  printf -- "Usage: %s <coralnpu_m3_sim> <crt0_vector_test.elf>\n" "$0"
  exit 1
fi

sim_bin="$(realpath "$1")"
elf_file="$(realpath "$2")"

output="$("$sim_bin" --semihost_htif --allow_memory_region=0x1000:0x4000000:rx --allow_memory_region=0x4001000:0x4000000:rw "$elf_file" 2>&1)"
exit_code=$?

printf -- "--- SIMULATOR OUTPUT ---\n"
printf "%s\n" "$output"
printf -- "--- END SIMULATOR OUTPUT ---\n"

if [[ $exit_code -ne 0 ]]; then
  printf -- "crt0_vector_test failed with exit code: %s\n" "$exit_code"
  exit 1
fi

if ! printf "%s" "$output" | grep -q "mpause instruction received"; then
  printf -- "Mpause not received.\n"
  exit 1
fi

printf -- "crt0_vector_test passed.\n"
exit 0
