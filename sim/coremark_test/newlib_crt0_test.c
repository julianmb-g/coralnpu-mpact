// Copyright 2026 Google LLC
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>

int main() {
    // ADR 006: Attempt to dereference a null pointer to trigger a trap.
    // This tests that the default startup without a proper crt0.S
    // will result in a predictable failure.
    int *null_ptr = (int *)0;
    volatile int val = *null_ptr; // Dereferencing null should cause a trap

    // Should not be reached
    printf("Test failed: Dereferencing null did not cause a trap. Value: %d\n", val);
    return 1;
}
