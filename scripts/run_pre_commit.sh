#!/bin/bash
# This script runs on the HOST. It starts the podman container and runs the validation script.

set -e

REPO_ROOT=$(git rev-parse --show-toplevel)
IMAGE=${PODMAN_IMAGE:-"localhost/coralnpu-build"}

# Find siblings
MPACT_RISCV_DIR="${REPO_ROOT}/../julianmb-g_mpact-riscv"
MPACT_SIM_DIR="${REPO_ROOT}/../julianmb-g_mpact-sim"

# Fallbacks
if [ ! -d "${MPACT_RISCV_DIR}" ]; then
  MPACT_RISCV_DIR="${REPO_ROOT}/../mpact-riscv"
fi
if [ ! -d "${MPACT_SIM_DIR}" ]; then
  MPACT_SIM_DIR="${REPO_ROOT}/../mpact-sim"
fi

echo "Running pre-commit validation in Podman container using image: ${IMAGE}..."

# Mount this repo to /src, and siblings to /mpact-riscv and /mpact-sim
# Use --override_module because we are now using Bzlmod.
# Use --userns=keep-id:uid=1000,gid=1000 to avoid root ownership issues
podman run --rm \
  --network=host \
  --userns=keep-id:uid=1000,gid=1000 \
  -v "${REPO_ROOT}:/src:ro" \
  -v "${MPACT_RISCV_DIR}:/mpact-riscv:ro" \
  -v "${MPACT_SIM_DIR}:/mpact-sim:ro" \
  -e BAZEL_BUILD_FLAGS="${BAZEL_BUILD_FLAGS} --override_module=mpact-riscv=/mpact-riscv --override_module=mpact-sim=/mpact-sim --build_tag_filters=-local,-requires-network" \
  -e BAZEL_TEST_FLAGS="${BAZEL_TEST_FLAGS} --override_module=mpact-riscv=/mpact-riscv --override_module=mpact-sim=/mpact-sim --test_tag_filters=-local,-requires-network" \
  -w /src \
  "${IMAGE}" \
  bash /src/scripts/pre_commit_validation.sh
