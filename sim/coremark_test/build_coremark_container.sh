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
set -e

# Script to build the coremark-builder Podman image.
# This script is run outside of Blaze to avoid hermeticity conflicts.

# Define the Dockerfile location
dockerfile="coremark_builder.Dockerfile"
# Define the output tarball location
output_tar="/tmp/coremark-builder.tar"

echo "Building coremark-builder image..."
podman build -t coremark-builder:latest -f "$(dirname "$0")/$dockerfile" "$(dirname "$0")"

echo "Saving coremark-builder image to $output_tar..."
podman save coremark-builder:latest > "$output_tar"

echo "Coremark builder image built and saved to $output_tar."
