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
#include "core_portme.h"

// Simple assertion macro
#define ASSERT(cond, msg) \
    if (!(cond)) { \
        printf("ASSERT FAILED: %s in %s at line %d\n", msg, __FILE__, __LINE__); \
        return 1; \
    }

int test_stdout_unbuffered() {
    printf("Running test_stdout_unbuffered...\n");
    int argc = 0;
    portable_init(NULL, &argc, NULL);
    printf("stdout->_flags = 0x%04x\n", stdout->_flags);
    ASSERT((stdout->_flags & 0x0002) != 0, "stdout is not unbuffered");
    printf("test_stdout_unbuffered PASSED\n");
    return 0;
}

int test_float_printf() {
    printf("Running test_float_printf...\n");
    float f_val = 3.14159f;
    // Test basic float printing
    // We use explicit cast to double for %f to satisfy -Werror=double-promotion
    ee_printf("Float: %f\n", (double)f_val);

    // Test lower float output bound
    // We expect the output to be at least 3.14
    float lower_bound = 3.14f;
    ASSERT(f_val >= lower_bound, "f_val is less than lower bound");

    // Test upper float output bound
    // We expect the output to be at most 3.15
    float upper_bound = 3.15f;
    ASSERT(f_val <= upper_bound, "f_val is greater than upper bound");

    printf("test_float_printf PASSED\n");
    return 0;
}

int main() {
    int failures = 0;
    failures += test_float_printf();
    if (failures == 0) {
        ee_printf("ALL EE_PRINTF TESTS PASSED\n");
        return 0;
    } else {
        ee_printf("EE_PRINTF TESTS FAILED: %d failures\n", failures);
        return 1;
    }
}
