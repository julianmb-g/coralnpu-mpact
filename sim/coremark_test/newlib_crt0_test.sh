#!/bin/bash
# Copyright 2026 Google LLC

# Hermetic dynamic compilation and execution logic utilizing newlib's standard crt0

if [ "$#" -ne 4 ]; then
  echo "Usage: $0 <coremark_unified.S> <linker.ld> <coralnpu_m3_sim> <image_tar>"
  exit 1
fi

ASSEMBLY_FILE=$(realpath "$1")
LINKER_SCRIPT=$(realpath "$2")
SIM_BIN=$(realpath "$3")
IMAGE_TAR=$(realpath "$4")

podman load -i "$IMAGE_TAR"

if [ ! -f "$ASSEMBLY_FILE" ] || [ ! -f "$LINKER_SCRIPT" ]; then
  echo "Error: One or more input files not found."
  exit 1
fi

TMP_DIR=$(mktemp -d) || exit 1
trap 'rm -rf "$TMP_DIR"' EXIT

cp "$ASSEMBLY_FILE" "$TMP_DIR/local_coremark_unified.S"
cp "$LINKER_SCRIPT" "$TMP_DIR/local_linker.ld"

echo "Compiling dynamically with newlib crt0..."
podman run --userns=keep-id:uid=1000,gid=1000 --rm -v "$TMP_DIR":/workspace -w /workspace coremark-builder:latest sh -c '
riscv-none-elf-gcc -c -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f local_coremark_unified.S -o local_coremark_unified.o &&
riscv-none-elf-gcc -fno-exceptions -fno-builtin -fno-isolate-erroneous-paths-dereference -fno-isolate-erroneous-paths-attribute -fno-delete-null-pointer-checks -u _printf_float -Wno-attributes -Tlocal_linker.ld -Wl,-Map,/workspace/coremark_unified_tmp.map -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f -O3 -ftree-vectorize -fno-vect-cost-model -ffast-math -ffp-contract=off local_coremark_unified.o -o /workspace/coremark_unified_tmp.elf -lm
'

if [ ! -f "$TMP_DIR/coremark_unified_tmp.elf" ]; then
  echo "Compilation failed."
  exit 1
fi

echo "Compilation successful. Performing dynamic verification of newlib crt0 incompatibility..."

# Create a minimal pty_runner to capture simulator output
cat << 'PYEOF' > "$TMP_DIR/pty_runner.py"
import pty, os, sys, subprocess
master, slave = pty.openpty()
try:
    p = subprocess.Popen(sys.argv[1:], stdin=slave, stdout=slave, stderr=slave, close_fds=True)
except OSError as e:
    sys.exit(1)
os.close(slave)
try:
    while True:
        data = os.read(master, 1024)
        if not data: break
        sys.stdout.buffer.write(data)
        sys.stdout.flush()
except OSError: pass
p.wait()
sys.exit(p.returncode)
PYEOF

echo "Running ELF in simulator (EXPECTED TO FAIL)..."
SIM_OUTPUT=$(timeout 30s python3 "$TMP_DIR/pty_runner.py" "$SIM_BIN" --semihost_htif "$TMP_DIR/coremark_unified_tmp.elf" 2>&1)
SIM_EXIT=$?

echo "--- SIMULATOR OUTPUT ---"
echo "$SIM_OUTPUT"
echo "--- END SIMULATOR OUTPUT ---"

# The standard newlib crt0 does not enable FPU/Vector units in mstatus CSR at _start,
# which causes an InstructionAccessFault or similar illegal instruction fault when
# the first FPU/Vector instruction is encountered.
if echo "$SIM_OUTPUT" | grep -q "InstructionAccessFault" || [ $SIM_EXIT -ne 0 ]; then
  echo "Dynamic verification SUCCESSFUL: Simulator detected expected failure (Exit: $SIM_EXIT) proving standard newlib crt0 is incompatible."
  exit 0
else
  echo "Error: Dynamic verification failed. Simulator did not detect InstructionAccessFault and returned exit code 0!"
  exit 1
fi
