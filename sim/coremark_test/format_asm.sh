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
# format_asm.sh
# ADR 012: Enforce assembly formatting constraints

if [ -z "$1" ]; then
    echo "Usage: $0 <assembly_file.S>"
    exit 1
fi

python3 -c '
import sys
import re

input_file = sys.argv[1]
with open(input_file, "r") as f:
    lines = f.readlines()

formatted = []
for line in lines:
    stripped = line.strip()
    if not stripped:
        formatted.append(line)
        continue
    # Comments: Retain at zero indentation
    if re.match(r"^(#|//|;)", stripped):
        formatted.append(stripped + "\n")
    # Dot-prefixed directives: Zero leading spaces (ADR 012)
    elif re.match(r"^\.", stripped):
        formatted.append(stripped + "\n")
    # Labels: Zero leading spaces (ADR 012)
    elif re.match(r"^[a-zA-Z0-9_]+:$", stripped):
        formatted.append(stripped + "\n")
    # Instructions: Exactly two leading spaces
    elif re.match(r"^[a-zA-Z]", stripped):
        formatted.append("  " + stripped + "\n")
    else:
        # Keep other lines as is
        formatted.append(line)

with open(input_file, "w") as f:
    f.writelines(formatted)
' "$1"