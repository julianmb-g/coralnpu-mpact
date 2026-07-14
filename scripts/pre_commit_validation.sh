#!/bin/bash
set -e

echo "=== Copying workspace to temporary directory ==="
# We copy /src (which is read-only mount of repo) to /dev/shm/workspace
# to allow writable build.
mkdir -p /dev/shm/workspace
cp -a /src/. /dev/shm/workspace/

cd /dev/shm/workspace

# Ensure clean state in the copy
if [ -d .git ]; then
  git clean -xfd
fi

echo "=== Running bazel build ==="
bazel build ${BAZEL_BUILD_FLAGS} -- //... -//sim/coremark_test/...

echo "=== Running bazel test ==="
bazel test ${BAZEL_TEST_FLAGS} --test_output=errors -- //... -//sim/coremark_test/...

echo "=== Running bazel test for coremark_test ==="
bazel test ${BAZEL_BUILD_FLAGS} ${BAZEL_TEST_FLAGS} --build_tests_only --test_output=errors -- //sim/coremark_test/... || [ $? -eq 4 ]
