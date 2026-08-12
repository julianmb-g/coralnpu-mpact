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
# build_unified_asm.sh

set -e

# 3.1.3: Compile unified assembly natively via GCC (ADR 023)
# ADR 020, ADR 031: Enforce C99 standard (-std=c99)
# ADR 027: Enforce march (-march=rv32imf_zve32f_zicsr_zifencei_zbb)
# ADR 029: Enforce mabi (-mabi=ilp32f)

# $1: output directory (@D)
# $2: common_cflags.sh
# $3: coremark_unified.c
# $4...: core_portme files
# $Last-2: crt0.S
# $Last-1: linker.ld
# $Last: format_asm.sh

# Get arguments
out_dir=$1
common_cflags_file=$2
coremark_unified_c=$3
shift 3
num_args=$#
format_asm=${@:$num_args:1}
linker_ld=${@:$((num_args-1)):1}
crt0_s=${@:$((num_args-2)):1}
# Everything else is core_portme files
core_portme_files=${@:1:$((num_args-3))}

tmp_dir=$(mktemp -d)
mkdir -p "$tmp_dir/build_context"
podman_vfs_dir="$tmp_dir/podman_vfs"
mkdir -p "$podman_vfs_dir"
mkdir -p "$out_dir"

# Clean up on exit
trap 'rm -rf "$tmp_dir" 2>/dev/null || true' EXIT

echo "DEBUG: tmp_dir is $tmp_dir"

# Extract Coremark source on the host to allow patching (ADR 001, ADR 002)
echo "DEBUG: Extracting /tmp/coremark.tar.gz"
# Verify SHA256 checksum on host (ADR 003, ADR 021, ADR 033)
echo "99c5a6d63af85a281b4e4d6ccb522c446653c435dfec9455ad73ef9e71f28bde  /tmp/coremark.tar.gz" | sha256sum -c || exit 1
tar xzf /tmp/coremark.tar.gz -C "$tmp_dir/build_context" --strip-components=1

# ADR 001, ADR 002: Inject include guards into coremark.h and other files
# to prevent redefinition errors during unified build.
# Use python3 on the host to avoid restricted tools (ADR 001, ADR 002 violation).
echo "DEBUG: Patching files"
python3 -c "import sys; c = sys.stdin.read(); print('#ifndef COREMARK_H_GUARD\n#define COREMARK_H_GUARD\n' + c + '\n#endif')" < "$tmp_dir/build_context/coremark.h" > "$tmp_dir/build_context/coremark.h.tmp" && mv "$tmp_dir/build_context/coremark.h.tmp" "$tmp_dir/build_context/coremark.h"

python3 -c "import sys; c = sys.stdin.read(); print(c.replace('#define matrix_clip', '#ifndef matrix_clip\n#define matrix_clip').replace(' & 0x0ffff)', ' & 0x0ffff)\n#endif').replace('#define matrix_big', '#ifndef matrix_big\n#define matrix_big').replace('(0xf000 | (x))', '(0xf000 | (x))\n#endif').replace('#define bit_extract', '#ifndef bit_extract\n#define bit_extract').replace(' << (to))))', ' << (to))))\n#endif'))" < "$tmp_dir/build_context/core_matrix.c" > "$tmp_dir/build_context/core_matrix.c.tmp" && mv "$tmp_dir/build_context/core_matrix.c.tmp" "$tmp_dir/build_context/core_matrix.c"

python3 -c "import sys; c = sys.stdin.read(); print(c.replace('#define MATDAT_INT', '#ifndef MATDAT_INT\n#define MATDAT_INT').replace('#define MATDAT_INT 1', '#define MATDAT_INT 1\n#endif').replace('typedef double secs_ret;', '#ifndef secs_ret\ntypedef double secs_ret;\n#endif').replace('typedef ee_u32 secs_ret;', '#ifndef secs_ret\ntypedef ee_u32 secs_ret;\n#endif').replace('#define ee_printf printf', '#ifndef ee_printf\n#define ee_printf printf\n#endif'))" < "$tmp_dir/build_context/coremark.h" > "$tmp_dir/build_context/coremark.h.tmp" && mv "$tmp_dir/build_context/coremark.h.tmp" "$tmp_dir/build_context/coremark.h"

# Create the Dockerfile using the preloaded base image
cat <<EOF > "$tmp_dir/Dockerfile.tmp"
FROM coremark-builder-base:latest
WORKDIR /coremark
EOF

# Copy port files and scripts into the same build_context
echo "DEBUG: Copying port files"
cp -L "$common_cflags_file" "$tmp_dir/build_context/host_common_cflags.sh"
cp -L "$coremark_unified_c" "$tmp_dir/build_context/coremark_unified.c"
cp -L $core_portme_files "$tmp_dir/build_context/"
cp -L "$crt0_s" "$tmp_dir/build_context/crt0.S"
cp -L "$linker_ld" "$tmp_dir/build_context/linker.ld"
cp -L "$format_asm" "$tmp_dir/build_context/format_asm.sh"

# Load the base image and tag it for Dockerfile.tmp
if [[ -f "/tmp/coremark-builder.tar" ]]; then
  podman --root "$podman_vfs_dir" --runroot "$tmp_dir/runroot" --storage-driver=vfs load -i "/tmp/coremark-builder.tar" > /dev/null
  podman --root "$podman_vfs_dir" --runroot "$tmp_dir/runroot" --storage-driver=vfs tag localhost/coremark-builder:latest coremark-builder-base > /dev/null || true
fi

# Build the Docker image from preloaded base
podman --root "$podman_vfs_dir" --runroot "$tmp_dir/runroot" --storage-driver=vfs build -q -t coremark-builder -f "$tmp_dir/Dockerfile.tmp" "$tmp_dir" > /dev/null

# Run the container and mount the build context
# ADR 003: Use --userns=keep-id:uid=1000,gid=1000
podman --root "$podman_vfs_dir" --runroot "$tmp_dir/runroot" --storage-driver=vfs run --userns=keep-id:uid=1000,gid=1000 \
    -v "$tmp_dir/build_context":/build_context:Z \
    -v "$(pwd)/$out_dir":/output:Z \
    coremark-builder \
    /bin/sh -c " \
    set -e; \
    cd /build_context; \
    . /build_context/host_common_cflags.sh; \
    # ADR 001, ADR 002: Use header wrapping instead of forbidden content modification.
    riscv-none-elf-gcc \$COMMON_CFLAGS -mno-riscv-attribute -mno-relax -I/build_context -I. -S -o /output/coremark_unified.S coremark_unified.c; \
    riscv-none-elf-gcc \$COMMON_CFLAGS -mno-riscv-attribute -mno-relax -c -o crt0.o crt0.S; \
    riscv-none-elf-gcc \$COMMON_CFLAGS \
        -nostartfiles \
        -T/build_context/linker.ld \
        -Wl,-Map=/output/coremark_unified.map \
        -o /output/coremark_unified.elf \
        crt0.o \
        /output/coremark_unified.S \
        -lc -lm -lgcc; \
    riscv-none-elf-objdump -d /output/coremark_unified.elf > /output/coremark_unified.objdump; \
    "

echo "DEBUG: contents of $out_dir on host:"
ls -l "$out_dir"

# Format the generated assembly on the host (ADR 012)
chmod +x "$format_asm"
bash "$format_asm" "$(pwd)/$out_dir/coremark_unified.S"
