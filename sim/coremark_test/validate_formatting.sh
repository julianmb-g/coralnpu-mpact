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
for input_file in "$@"; do
  # Validate that no shell variables are uppercase (ADR 032), exempting global constants like COMMON_CFLAGS
  if grep -E '^[[:space:]]*[A-Z_]+=' "$input_file" | grep -qvE '^[[:space:]]*COMMON_CFLAGS='; then
    printf "Error: Variable name in %s must be lowercase (ADR 032).\n" "$input_file"
    exit 1
  fi

  if [[ ! -f "$input_file" ]]; then
    printf "Error: File %s not found.\n" "$input_file"
    exit 1
  fi

  if [[ "$input_file" == *.sh ]]; then
    printf "Formatting validation passed for %s.\n" "$input_file"
    continue
  fi

  printf "Validating formatting for: %s\n" "$input_file"
  awk '
BEGIN { error_count = 0 }
{
  line = $0
  # Skip empty or whitespace-only lines
  if (line ~ /^[[:space:]]*$/) next

  # Skip comments
  if (line ~ /^[[:space:]]*#/ || line ~ /^[[:space:]]*\/\//) next

  # Trim leading whitespace for classification
  trimmed = line
  sub(/^[[:space:]]+/, "", trimmed)

  expected_indent = 2 # Default to 2 spaces (Instructions, etc.)

  if (trimmed ~ /^\./) {
    # It is a directive (e.g., .text, .word, .insn)
    expected_indent = 0
  } else if (trimmed ~ /^[a-zA-Z0-9_.]+:/) {
    # It is a label
    expected_indent = 0
  }

  # Check actual indentation
  if (expected_indent == 0) {
    if (line ~ /^[[:space:]]+/) {
      print "Line " NR ": Expected 0 indentation, found leading whitespace."
      print "  Line: [" line "]"
      error_count++
    }
  } else if (expected_indent == 2) {
    if (line !~ /^  [^[:space:]]/) {
      print "Line " NR ": Expected exactly 2 leading spaces."
      print "  Line: [" line "]"
      error_count++
    }
  }
}
END {
  if (error_count > 0) {
    print "Formatting validation failed for " FILENAME " with " error_count " errors."
    exit 1
  } else {
    print "Formatting validation passed for " FILENAME "."
    exit 0
  }
}
' "$input_file"
  if [[ $? -ne 0 ]]; then
    exit 1
  fi
done
# Cache buster 2026-08-08-2

