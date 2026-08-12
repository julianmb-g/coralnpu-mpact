#!/bin/bash
# Copyright 2026 Google LLC
set -e

# Locate target directory
target_dir=""
# Try to follow symlinks to find original workspace
real_path=$(readlink -f "$0" 2>/dev/null || echo "$0")
real_dir=$(dirname "$real_path")

# If running inside a Blaze snapshot, try to resolve to active workspace
if [[ "$real_dir" =~ ^(.*)/\.snapshot/[0-9]+/(.*)$ ]]; then
  real_dir="${BASH_REMATCH[1]}/${BASH_REMATCH[2]}"
fi

for path in "$real_dir" "${TEST_SRCDIR}/google3/sim/coremark_test" "sim/coremark_test" "$(dirname "$0")"; do
  if [[ -d "$path" ]] && [[ -f "$path/BUILD" ]]; then
    target_dir="$path"
    break
  fi
done

if [[ -z "$target_dir" ]]; then
  printf "Error: Target directory not found.\n"
  exit 1
fi

test_files_dir="$(realpath "$target_dir/../test/testfiles")"

printf "Checking for mandatory artifacts in %s...\n" "$test_files_dir"
required_artifacts=("coremark_unified.map" "coremark_unified.S")
for artifact in "${required_artifacts[@]}"; do
  if [[ ! -f "$test_files_dir/$artifact" ]]; then
    printf "Error: Mandatory artifact %s missing in %s\n" "$artifact" "$test_files_dir"
    exit 1
  fi
done

# Check for stale detritus in target_dir
printf "Checking for stale detritus in %s...\n" "$target_dir"
target_detritus_patterns=(
  "*.tmp"
  "*.log"
  "*.out"
  "*.o"
  "*.d"
  "*.ii"
  ".git/index.lock"
  ".hg/store/lock"
)
for pattern in "${target_detritus_patterns[@]}"; do
  if find "$target_dir" -type f -name "$pattern" | grep -q .; then
    printf "Error: Found stale detritus in %s matching pattern '%s':\n" "$target_dir" "$pattern"
    find "$target_dir" -type f -name "$pattern"
    exit 1
  fi
done

printf "Checking for unexpected files in %s...\n" "$test_files_dir"
# Define expected extensions that are allowed
allowed_extensions=("S" "c" "h" "ld" "elf" "map" "sh" "py" "disassm" "bin" "awk")
# required_artifacts are already defined

# Find all files in the testfiles directory
all_files=$(find "$test_files_dir" -type f)

for file in $all_files; do
  basename_file=$(basename "$file")
  is_required=false
  for required in "${required_artifacts[@]}"; do
    if [[ "$basename_file" == "$required" ]]; then
      is_required=true
      break
    fi
  done

  if [[ "$is_required" == false ]]; then
    ext="${file##*.}"
    if [[ ! " ${allowed_extensions[@]} " =~ " ${ext} " ]]; then
      printf "Error: Found unexpected artifact: %s (Extension: %s)\n" "$file" "$ext"
      exit 1
    fi
  fi
done

printf "Workspace clean validation passed.\n"
exit 0
