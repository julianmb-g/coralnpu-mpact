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
set -ex

# Define the expected path for synchronized artifacts in the workspace
testfiles_dir="sim/test/testfiles"

# 1. 210.31b.1a & 210.31b.2a: Verify files exist in the runfiles tree robustly
required_files=(
    "coremark_unified.S"
    "coremark_unified.map"
    "crt0.S"
    "linker.ld"
)

for file in "${required_files[@]}"; do
    if ! find . -name "${file}" | grep -q "."; then
        echo "ERROR: Artifact missing in runfiles: ${file}"
        exit 1
    fi
done

# 2. 210.31b.3a: Verify coremark_unified.map is NOT tracked by VCS (Mercurial or Git)
if [ -d ".hg" ] && command -v hg >/dev/null 2>&1; then
    if hg files "${testfiles_dir}/coremark_unified.map" >/dev/null 2>&1; then
        echo "ERROR: coremark_unified.map is tracked by Mercurial!"
        exit 1
    fi
fi

if [ -d ".git" ] && command -v git >/dev/null 2>&1; then
    if git ls-files --error-unmatch "${testfiles_dir}/coremark_unified.map" >/dev/null 2>&1; then
        echo "ERROR: coremark_unified.map is tracked by Git!"
        exit 1
    fi
fi

# We only run the deep content checks if we are running in the workspace on host,
# or if the map file is present in the current directory tree (e.g. locally).
map_file=$(find . -name "coremark_unified.map" | head -n 1)
if [ -n "${map_file}" -a -f "${map_file}" ]; then
    # 3. 210.31b.4: Verify start address of .text segment is at 0x1000 in coremark_unified.map
    # Typical map file format: .text           0x00001000     0x5fc0
    if ! grep -qE "^\.text[[:space:]]+0x(00000000)?00001000" "${map_file}"; then
        echo "ERROR: .text segment does not start at 0x1000 in coremark_unified.map!"
        exit 1
    fi

    # 4. 210.31b.5: Verify presence of main and _start symbols in coremark_unified.map
    for sym in "main" "_start"; do
        if ! grep -qE "\b${sym}\b" "${map_file}"; then
            echo "ERROR: Symbol ${sym} missing from coremark_unified.map"
            exit 1
        fi
    done
fi

# 5. 212.34a/b: Verify local CoreMark source files absence (ADR 002)
if find . -maxdepth 4 -name "core_main.c" | grep -q "core_main.c"; then
    echo "ERROR: Local CoreMark source files detected! (ADR 002 violation)"
    exit 1
fi

# 6. 212.35a/b: Verify duplicate port directory absence
if [ -d "sim/coremark_test/coralnpu_port" ]; then
    echo "ERROR: Duplicate port directory 'coralnpu_port' detected!"
    exit 1
fi

# 7. 212.36a/b: Verify root build artifacts absence
forbidden_root_artifacts=(
    "coremark_unified.S"
    "coremark_unified.map"
    "coremark_unified.elf"
)
for art in "${forbidden_root_artifacts[@]}"; do
    if [ -f "${art}" ]; then
        echo "ERROR: Forbidden build artifact found in root: ${art}"
        exit 1
    fi
done

# 8. 212.37a/b: Verify testfiles ELF absence (Definition of Done)
if [ -f "${testfiles_dir}/coremark_unified.elf" ]; then
    echo "ERROR: Transient ELF file found in testfiles! (Hygiene violation)"
    exit 1
fi

# 9. Verify Hardware Utilization via scan_asm.sh (ADR 030)
asm_file=$(find . -name "coremark_unified.S" | head -n 1)
if [ -n "${asm_file}" -a -f "${asm_file}" ]; then
    scan_script="sim/coremark_test/scan_asm.sh"
    if [ -f "${scan_script}" ]; then
        if ! bash "${scan_script}" "${asm_file}"; then
            echo "ERROR: scan_asm.sh failed on ${asm_file}"
            exit 1
        fi
    else
        echo "ERROR: scan_asm.sh not found."
        exit 1
    fi
fi

echo "verify_artifacts.sh PASSED"
exit 0
