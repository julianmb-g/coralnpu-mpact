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

# Arguments:
# $1: C/C++ source file (e.g. crt0_mstatus_test.c, crt0_vector_test.c, test_ee_printf.c, or portable_malloc_test.c)
# $2: linker.ld
# $3: crt0.S
# $4: core_portme.c
# $5: core_portme.h
# $6: common_cflags.sh
# $7: output elf
# $8: output map
# $9: output objdump

src_file=$1
linker_ld=$2
crt0_s=$3
core_portme_c=$4
core_portme_h=$5
common_cflags=$6
out_elf=$7
out_map=$8
out_objdump=$9
extra_flags=${10:-}

tmp_dir=$(mktemp -d)
podman_vfs_dir="$tmp_dir/podman_vfs"
mkdir -p "$podman_vfs_dir"
trap "rm -rf linker.ld crt0.S core_portme.c core_portme.h common_cflags.sh test_src.c crt0.o core_portme.o test_src.o test_tmp.elf test_tmp.map test_tmp.objdump coremark_authentic.h coremark.h; podman unshare rm -rf \"$tmp_dir\" 2>/dev/null || true" EXIT

cp -L "$linker_ld" linker.ld
cp -L "$crt0_s" crt0.S
cp -L "$core_portme_c" core_portme.c
cp -L "$core_portme_h" core_portme.h
cp -L "$common_cflags" common_cflags.sh
cp -L "$src_file" test_src.c

# Inject mock coremark_authentic.h and coremark.h for port compilation compatibility
cat > coremark_authentic.h <<EOF
#ifndef COREMARK_AUTHENTIC_H
#define COREMARK_AUTHENTIC_H
#include <stdint.h>
#define SEED_ARG 0
#define SEED_FUNC 1
#define SEED_VOLATILE 2
#include "core_portme.h"
#ifndef secs_ret
typedef float secs_ret;
#define secs_ret secs_ret
#endif
#define MATDAT_INT 0
#endif
EOF

cat > coremark.h <<EOF
#include "coremark_authentic.h"
EOF

# Load the Podman image robustly if /tmp/coremark-builder.tar is present
if [[ -f "/tmp/coremark-builder.tar" ]]; then
  podman --root "$podman_vfs_dir" --runroot "$tmp_dir/runroot" --storage-driver=vfs load -i "/tmp/coremark-builder.tar"
fi

podman --root "$podman_vfs_dir" --runroot "$tmp_dir/runroot" --storage-driver=vfs run --userns=keep-id:uid=1000,gid=1000 --rm --env EXTRA_FLAGS="$extra_flags" -v "$(pwd)":/workspace -w /workspace coremark-builder:latest sh -c '
. /workspace/common_cflags.sh
COMMON_CFLAGS="$COMMON_CFLAGS $EXTRA_FLAGS"
# We need to use -mno-relax to preserve gp and sp loading correctly!
riscv-none-elf-gcc -c $COMMON_CFLAGS -mno-relax -I/workspace /workspace/crt0.S -o /workspace/crt0.o
riscv-none-elf-gcc -c $COMMON_CFLAGS -mno-relax -I/workspace /workspace/core_portme.c -o /workspace/core_portme.o
riscv-none-elf-gcc -c $COMMON_CFLAGS -mno-relax -I/workspace /workspace/test_src.c -o /workspace/test_src.o

if grep -q "newlib_crt0_test" /workspace/test_src.c; then
  riscv-none-elf-gcc -T/workspace/linker.ld -Wl,-Map,/workspace/test_tmp.map $COMMON_CFLAGS -mno-relax /workspace/core_portme.o /workspace/test_src.o -o /workspace/test_tmp.elf -lc -lm -lgcc
else
  riscv-none-elf-gcc -nostartfiles -T/workspace/linker.ld -Wl,-Map,/workspace/test_tmp.map $COMMON_CFLAGS -mno-relax /workspace/crt0.o /workspace/core_portme.o /workspace/test_src.o -o /workspace/test_tmp.elf -lc -lm -lgcc
fi
riscv-none-elf-objdump -d /workspace/test_tmp.elf > /workspace/test_tmp.objdump
'

mv test_tmp.elf "$out_elf"
mv test_tmp.map "$out_map"
mv test_tmp.objdump "$out_objdump"
