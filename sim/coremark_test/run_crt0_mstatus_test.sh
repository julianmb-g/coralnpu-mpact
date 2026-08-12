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
# run_crt0_mstatus_test.sh

set -e

# Path to the simulator and ELF, relative to current directory in test execution
sim_path="sim/coralnpu_m3_sim"
elf_path="sim/coremark_test/crt0_mstatus_test.elf"

# Run the simulator with semihosting and appropriate memory regions
"${sim_path}" --semihost_htif --allow_memory_region=0x1000:0x4000000:rx --allow_memory_region=0x4001000:0x4000000:rw "${elf_path}"
