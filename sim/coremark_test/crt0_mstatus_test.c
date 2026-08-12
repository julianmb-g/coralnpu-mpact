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

#include <stdint.h>

// Simple assertion macro
#define ASSERT(cond, msg) \
    if (!(cond)) { \
        return 1; \
    }

int main() {
    uint32_t mstatus_val;
    // Read mstatus CSR
    __asm__ volatile ("csrr %0, mstatus" : "=r"(mstatus_val));

    // Expected mstatus: MPP = 11 (Machine mode), FS=10 (Clean), VS=10 (Clean). Value: 0x5C00
    uint32_t expected_mstatus = 0x5C00;
    ASSERT(mstatus_val == expected_mstatus, "mstatus not initialized correctly");

    return 0;
}
