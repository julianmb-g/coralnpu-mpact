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
# run_portable_malloc_test.sh

set -e

# Paths relative to current directory in test execution
sim_path="sim/coralnpu_m3_sim"
elf_path="sim/coremark_test/portable_malloc_test.elf"
validation_sh="sim/coremark_test/portable_malloc_test.sh"

bash "${validation_sh}" "${sim_path}" "${elf_path}"
