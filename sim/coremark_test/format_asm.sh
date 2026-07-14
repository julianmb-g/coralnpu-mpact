#!/bin/sh
set -e

# Arguments:
# $1: Input assembly file
# $2: Output assembly file

input_asm=$1
output_asm=$2

awk '
{
  # Remove leading whitespace
  sub(/^[[:space:]]+/, "");

  # Check if line should not be indented:
  # 1. Empty line
  # 2. Dot directive (e.g., .text), but NOT .word or .insn (which are instructions/data)
  # 3. Label definition (ends with :)
  if ($0 == "" || 
      (substr($0, 1, 1) == "." && $0 !~ /^\.word[[:space:]]+0x/ && $0 !~ /^\.insn([[:space:]]|$)/) || 
      $0 ~ /^[a-zA-Z0-9_.]+:/) {
    print $0;
  } else {
    # Indent instructions by two spaces
    print "  " $0;
  }
}' "$input_asm" > "$output_asm"
