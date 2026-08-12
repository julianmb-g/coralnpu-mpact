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
# scan_asm.sh
# ADR 030: Automate Build-Time Assembly Verification

if [ -z "$1" ]; then
    echo "Usage: $0 <assembly_file.S>"
    exit 1
fi

input_file="$1"
failures=0

# 4.1.2.1: Verify active presence of hardware floating-point and vector instructions (ADR 030)
# Scan for fadd.s and fmul.s
fadd_s_count=$(grep -c "fadd.s" "${input_file}")
if [ "${fadd_s_count}" -eq 0 ]; then
    echo "ERROR: Missing fadd.s instruction in ${input_file}"
    failures=$((failures + 1))
fi
fmul_s_count=$(grep -c "fmul.s" "${input_file}")
if [ "${fmul_s_count}" -eq 0 ]; then
    echo "ERROR: Missing fmul.s instruction in ${input_file}"
    failures=$((failures + 1))
fi

# Scan for vadd (allow any variant)
vadd_count=$(grep -c "vadd\." "${input_file}")
if [ "${vadd_count}" -eq 0 ]; then
    echo "ERROR: Missing vadd.* instruction in ${input_file}"
    failures=$((failures + 1))
fi

# 4.1.2.2: Negative test cases (ADR 027, ADR 030)
# Ensure no double-precision float instructions (e.g., fadd.d)
if grep -qE "\b(fadd|fsub|fmul|fdiv|fmadd|fmsub|fcvt)\.d\b" "${input_file}"; then
    echo "ERROR: Double-precision instruction found in ${input_file}"
    failures=$((failures + 1))
fi

# 3.3.12: Forbid excessive fence.i instructions (ADR 028)
fence_i_count=$(grep -c "fence.i" "${input_file}")
if [ "${fence_i_count}" -gt 100 ]; then
    echo "ERROR: Excessive fence.i instructions found (${fence_i_count}). Max allowed: 100"
    failures=$((failures + 1))
fi

# 3.3.13: Ensure no software-emulated FP routines are called (ADR 028, ADR 030)
# Ensure no software float emulation calls (e.g., __float, __addsf3, __mulsf3)
if grep -qE "(__float|__addsf3|__mulsf3)" "${input_file}"; then
    echo "ERROR: Software float emulation call found in ${input_file}"
    failures=$((failures + 1))
fi

# 3.3.14: Forbid direct writes to read-only CSRs (vl and vtype) (ADR 031)
if grep -qE "csrw\s+(vl|vtype)" "${input_file}"; then
    echo "ERROR: Direct write to read-only CSR (vl or vtype) found in ${input_file}"
    failures=$((failures + 1))
fi

if [ "${failures}" -eq 0 ]; then
    echo "scan_asm.sh PASSED: All required float/vector instructions found and no forbidden instructions detected."
    exit 0
else
    echo "scan_asm.sh FAILED: ${failures} issues found."
    exit 1
fi
