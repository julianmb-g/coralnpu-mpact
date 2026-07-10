#!/bin/bash
set -e
if ! cmp -s "$1" "$2"; then
  echo "Error: The checked-in coremark_unified.S does not match the generated one."
  echo "Please run update.sh to update the checked-in file."
  diff -u "$2" "$1" || true
  exit 1
fi
if ! cmp -s "$3" "$4"; then
  echo "Error: The checked-in coremark_unified.map does not match the generated one."
  echo "Please run update.sh to update the checked-in file."
  diff -u "$4" "$3" || true
  exit 1
fi
echo "Files match."
exit 0
