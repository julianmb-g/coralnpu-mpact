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
# validate_build_tags.sh

if [[ -n "${TEST_SRCDIR}" ]]; then
    workspace="${TEST_WORKSPACE:-google3}"
    SCRIPT_DIR="${TEST_SRCDIR}/${workspace}/sim/coremark_test"
elif [[ -n "${TEST_BINARY}" ]]; then
    SCRIPT_DIR=$(dirname "${TEST_BINARY}")
else
    SCRIPT_DIR=$(dirname "$0")
fi

echo "DEBUG: TEST_SRCDIR=${TEST_SRCDIR}"
echo "DEBUG: TEST_WORKSPACE=${TEST_WORKSPACE}"
echo "DEBUG: SCRIPT_DIR=${SCRIPT_DIR}"

find_file() {
    local name="$1"
    if [[ -f "${SCRIPT_DIR}/${name}" ]]; then
        echo "${SCRIPT_DIR}/${name}"
    elif [[ -f "${SCRIPT_DIR}/../test/testfiles/${name}" ]]; then
        echo "${SCRIPT_DIR}/../test/testfiles/${name}"
    else
        local found
        found=$(find . -name "${name}" -print -quit 2>/dev/null)
        if [[ -f "${found}" ]]; then
            echo "${found}"
        else
            if [[ -n "${TEST_SRCDIR}" ]]; then
                found=$(find "${TEST_SRCDIR}" -name "${name}" -print -quit 2>/dev/null)
                if [[ -f "${found}" ]]; then
                    echo "${found}"
                    return 0
                fi
            fi
            echo "${name}"
        fi
    fi
}

build_file=$(find_file "BUILD")
build_asm_sh=$(find_file "build_unified_asm.sh")
gen_elf_sh=$(find_file "gen_coremark_elf.sh")
portme_h=$(find_file "core_portme.h")
core_portme_c=$(find_file "core_portme.c")
crt0_s=$(find_file "crt0.S")
common_cflags_sh=$(find_file "common_cflags.sh")
result=0

printf "Validating BUILD tags and pipeline flags...\n"

# 1. ADR 022: Mandate "local" tags on all tests to ensure local execution of host-side dependencies
all_tests=(
    "portable_malloc_test"
    "crt0_mstatus_test"
    "test_float_macros"
    "crt0_vector_test"
    "test_ee_printf"
    "newlib_crt0_test"
    "run_v2_vector_test"
    "run_final_test"
    "run_m4_test"
    "run_m4_vector_test"
    "verify_artifacts"
    "validate_formatting"
    "validate_build_tags"
)

for test_name in "${all_tests[@]}"; do
    if ! grep -A 15 "name = \"${test_name}\"" "$build_file" | grep -q "tags = \[\"local\"\]"; then
        printf -- "Error: Test '${test_name}' is missing required 'local' tag.\n"
        result=1
    fi
done

# 2. ADR 003: Verify --userns=keep-id in build scripts
if ! grep -q -e "--userns=keep-id:uid=1000,gid=1000" "$build_asm_sh"; then
    printf -- "Error: '--userns=keep-id:uid=1000,gid=1000' missing in $build_asm_sh\n"
    result=1
fi

# 3. ADR 028: Forbid linker tricks and subversive hacks in build scripts
subversive_regex="(--wrap|custom \.ld script hijacking|\.init_array payloads|LD_PRELOAD|-D[[:space:]]*[[:alnum:]_]+=[[:alnum:]_]+)"
if grep -qE "$subversive_regex" "$build_asm_sh" "$build_file"; then
    printf -- "Error: Unauthorized linker tricks or subversive hacks detected in build files.\n"
    result=1
fi

# 4. ADR 001, ADR 002: Forbid sed, awk, or patch commands in build pipeline
if grep -qE "(\bsed\b|\bawk\b|\bpatch\b|coremark\.patch)" "$build_asm_sh" "$build_file" "$gen_elf_sh"; then
    printf -- "Error: Unauthorized use of 'sed', 'awk', or 'patch' detected in build files.\n"
    result=1
fi

# 5. ADR 022: Forbid 'no-sandbox' tag and 'local' attribute for unified artifacts
if grep -A 50 "name = \"coremark_unified_artifacts\"" "$build_file" | grep -q "no-sandbox"; then
    printf -- "Error: 'coremark_unified_artifacts' has forbidden 'no-sandbox' tag.\n"
    result=1
fi
if ! grep -A 50 "name = \"coremark_unified_artifacts\"" "$build_file" | grep -q "\"local\""; then
    printf -- "Error: 'coremark_unified_artifacts' is missing the required '\"local\"' tag.\n"
    result=1
fi

# 6. ADR 001, ADR 002: Forbid grep -vE or similar filtering on authentic source files
if grep -qE "grep[[:space:]]+-vE" "$build_asm_sh"; then
    printf -- "Error: Unauthorized use of 'grep -vE' detected in build files.\n"
    result=1
fi

# 7. 210.15k.1b: Verify that any use of $COMMON_CFLAGS inside compile commands is backslash-escaped as \$COMMON_CFLAGS
if grep -E "riscv-none-elf-gcc" "$build_asm_sh" | grep -v "common_cflags" | grep -qE "[^\\]\\\$COMMON_CFLAGS"; then
    printf -- "Error: \$COMMON_CFLAGS is not properly escaped in $build_asm_sh compilation commands.\n"
    result=1
fi

# 8. 210.17i.2: Verify #define HAS_FLOAT 1 is defined in core_portme.h
if ! grep -qE "#define[[:space:]]+HAS_FLOAT[[:space:]]+1" "$portme_h"; then
    printf -- "Error: '#define HAS_FLOAT 1' is missing or incorrect in $portme_h\n"
    result=1
fi

# 9. 217.2, 217.3: Verify SHA256 checksum verification of the tarball (ADR 033)
if ! grep -qE "99c5a6d63af85a281b4e4d6ccb522c446653c435dfec9455ad73ef9e71f28bde" "$build_asm_sh"; then
    printf -- "Error: Missing or incorrect SHA256 checksum in $build_asm_sh\n"
    result=1
fi
if ! grep -qE "sha256sum -c" "$build_asm_sh"; then
    printf -- "Error: 'sha256sum -c' validation command missing in $build_asm_sh\n"
    result=1
fi

# 10. 212.38a/b: Verify legacy coralnpu_sim simulator usage absence
if grep -q "coralnpu_sim" "$build_file" "$build_asm_sh" "$gen_elf_sh"; then
    printf -- "Error: Deprecated 'coralnpu_sim' detected in build files. (Legacy target violation)\n"
    result=1
fi

# 11. ADR 008: Assert absence of fictional tohost_ready and fromhost_ready in core_portme.c
if grep -qE "(tohost_ready|fromhost_ready)" "$core_portme_c"; then
    printf -- "Error: Fictional 'tohost_ready' or 'fromhost_ready' detected in core_portme.c (ADR 008 violation).\n"
    result=1
fi

# 12. Phase 221: Assert presence of union-based Float-Int Cast translation layers in core_portme.h (ADR 001)
if ! grep -q "union" "$portme_h"; then
    printf -- "Error: Missing union-based Float-Int Cast translation layers in $portme_h.\n"
    result=1
fi

# 13. Assert presence of -mabi=ilp32f flag in build scripts (ADR 029)
if ! grep -q "\-mabi=ilp32f" "$build_asm_sh" && ! grep -q "\-mabi=ilp32f" "$common_cflags_sh" 2>/dev/null; then
    printf -- "Error: Missing -mabi=ilp32f compiler flag in build scripts.\n"
    result=1
fi

# 15. Assert ee_printf variadic declaration in core_portme.h (ADR 024)
if ! grep -q "void ee_printf(const char \*fmt, ...);" "$portme_h"; then
    printf -- "Error: Missing 'void ee_printf(const char *fmt, ...);' declaration in $portme_h.\n"
    result=1
fi

# 14. Phase 222: Assert absence of tohost_ready and fromhost_ready in crt0.S (ADR 008)
if grep -qE "(tohost_ready|fromhost_ready)" "$crt0_s"; then
    printf -- "Error: Fictional 'tohost_ready' or 'fromhost_ready' detected in crt0.S (ADR 008 violation).\n"
    result=1
fi

if [[ $result -ne 0 ]]; then
  printf -- "Build validation FAILED.\n"
  exit 1
fi

printf -- "Build validation PASSED.\n"
exit 0
