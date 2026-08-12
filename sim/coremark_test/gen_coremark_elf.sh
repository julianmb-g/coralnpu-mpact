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
set -e

# Arguments:
# $1: linker.ld
# $2: crt0.S
# $3: coremark_unified.S
# $4: build_unified_elf.sh
# $5: common_cflags.sh
# $6: output elf
# $7: output map
# $8: output objdump
# $9: location of coremark-builder.tar

linker_ld=$1
crt0_s=$2
coremark_unified_s=$3
build_unified_elf=$4
common_cflags=$5
out_elf=$6
out_map=$7
out_objdump=$8
coremark_builder_tar=$9

trap 'rm -rf local_linker.ld local_crt0.S local_coremark_unified.S local_build_unified_elf.sh local_common_cflags.sh local_crt0.o local_coremark_unified.o coremark_unified_tmp.elf coremark_unified_tmp.map coremark_unified_tmp.objdump' EXIT

cp -L "$linker_ld" local_linker.ld
cp -L "$crt0_s" local_crt0.S
cp -L "$coremark_unified_s" local_coremark_unified.S
cp -L "$build_unified_elf" local_build_unified_elf.sh
cp -L "$common_cflags" local_common_cflags.sh

# Load the Podman image robustly
if [[ -n "$coremark_builder_tar" && -f "$coremark_builder_tar" ]]; then
podman_vfs_dir="/tmp/podman_vfs_$$"
mkdir -p "$podman_vfs_dir"
trap "podman --root \"$podman_vfs_dir\" --storage-driver=vfs system reset -f 2>/dev/null || true; rm -rf \"$podman_vfs_dir\" 2>/dev/null || true" EXIT

  podman --root "$podman_vfs_dir" --storage-driver=vfs load -i "$coremark_builder_tar"
fi

# Verify the image is loaded
if ! podman --root "$podman_vfs_dir" --storage-driver=vfs images | grep -q "coremark-builder"; then
  if [[ -f "/tmp/coremark-builder.tar" ]]; then
    echo "Loading coremark-builder from /tmp/coremark-builder.tar..."
    podman --root "$podman_vfs_dir" --storage-driver=vfs load -i "/tmp/coremark-builder.tar"
  else
    echo "Error: coremark-builder image not found and /tmp/coremark-builder.tar does not exist."
    exit 1
  fi
fi

podman --root "$podman_vfs_dir" --storage-driver=vfs run --userns=keep-id:uid=1000,gid=1000 --rm -v "$(pwd)":/workspace -w /workspace coremark-builder:latest sh /workspace/local_build_unified_elf.sh /workspace/local_common_cflags.sh

mv coremark_unified_tmp.elf "$out_elf"
mv coremark_unified_tmp.map "$out_map"
mv coremark_unified_tmp.objdump "$out_objdump"
