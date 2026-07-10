#!/bin/bash

for INPUT_FILE in "$@"; do
  if [ ! -f "$INPUT_FILE" ]; then
    echo "Error: File $INPUT_FILE not found."
    exit 1
  fi

  echo "Validating formatting for: $INPUT_FILE"
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
    # It is a directive
    if (trimmed ~ /^\.word[[:space:]]+0x/ || trimmed ~ /^\.insn([[:space:]]|$)/) {
      expected_indent = 2
    } else {
      expected_indent = 0
    }
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
' "$INPUT_FILE"
  if [ $? -ne 0 ]; then
    exit 1
  fi
done
