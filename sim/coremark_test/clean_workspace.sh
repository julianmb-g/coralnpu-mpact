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
# clean_workspace.sh

set -e

# Target directories to clean
if [[ -z "${RUNFILES}" ]]; then
    workspace_root=$(git rev-parse --show-toplevel 2>/dev/null || hg root 2>/dev/null || echo ".")
    clean_dirs=(
        "${workspace_root}/sim/coremark_test"
        "${workspace_root}/sim/test/testfiles"
    )
else
    clean_dirs=(
        "${RUNFILES}/google3/sim/coremark_test"
        "${RUNFILES}/google3/sim/test/testfiles"
    )
fi

# 4.4.1 & 4.4.1.1: Delete stale log files (.log) and temporary files (.tmp)
for dir in "${clean_dirs[@]}"; do
    find "${dir}" -type f -name "*.log" -delete
    find "${dir}" -type f -name "*.tmp" -delete
done

echo "Workspace cleaned."
