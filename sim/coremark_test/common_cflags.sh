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
# common_cflags.sh

# ADR 027: Enforce march
COMMON_CFLAGS="-march=rv32imf_zve32f_zicsr_zifencei_zbb"
# ADR 029: Enforce mabi
COMMON_CFLAGS="${COMMON_CFLAGS} -mabi=ilp32f"
# Optimization and vectorization
COMMON_CFLAGS="${COMMON_CFLAGS} -O3 -ftree-vectorize -mstrict-align -ffast-math -ffp-contract=off -mrvv-vector-bits=zvl -fvect-cost-model=unlimited"
# Enable float (CoreMark) handled in core_portme.h
# ADR 020, ADR 031: Enforce C99 standard
COMMON_CFLAGS="${COMMON_CFLAGS} -std=c99"
# ADR 017, ADR 027: Stricter warning enforcement
COMMON_CFLAGS="${COMMON_CFLAGS} -Werror"
# ADR 017: No warning suppression flags

echo "${COMMON_CFLAGS}"
