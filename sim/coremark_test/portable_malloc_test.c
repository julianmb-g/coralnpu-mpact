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
#include <stdlib.h>
#include <string.h>

#include "core_portme.h"

// Simple assertion macro
#define ASSERT(cond, msg) \
    if (!(cond)) { \
        printf("ASSERT FAILED: %s in %s at line %d\n", msg, __FILE__, __LINE__); \
        return 1; \
    }

// Test case 1: Basic allocation and free
int test_basic_alloc_free() {
    printf("Running test_basic_alloc_free...\n");
    void *ptr = portable_malloc(100);
    ASSERT(ptr != NULL, "portable_malloc failed for basic allocation");
    portable_free(ptr);
    printf("test_basic_alloc_free PASSED\n");
    return 0;
}

// Test case 2: Multiple allocations
int test_multiple_allocs() {
    printf("Running test_multiple_allocs...\n");
    void *ptrs[5];
    for (int i = 0; i < 5; ++i) {
        ptrs[i] = portable_malloc(10 + i * 10);
        ASSERT(ptrs[i] != NULL, "portable_malloc failed for multiple allocations");
    }
    // Check if allocations are distinct (simplified check)
    for (int i = 0; i < 4; ++i) {
        for (int j = i + 1; j < 5; ++j) {
            ASSERT(ptrs[i] != ptrs[j], "Allocated blocks are not distinct");
        }
    }
    for (int i = 0; i < 5; ++i) {
        portable_free(ptrs[i]);
    }
    printf("test_multiple_allocs PASSED\n");
    return 0;
}

// Test case 3: Alignment test (e.g., 8-byte alignment)
int test_alignment() {
    printf("Running test_alignment...\n");
    void *ptr = portable_malloc(100);
    ASSERT(ptr != NULL, "portable_malloc failed for alignment test");
    // Assuming 8-byte alignment, check if address is a multiple of 8
    ASSERT(((unsigned long)ptr % 8) == 0, "Allocated address is not 8-byte aligned");
    portable_free(ptr);
    printf("test_alignment PASSED\n");
    return 0;
}

// Test case 4: Overflow test
int test_overflow() {
    printf("Running test_overflow...\n");
    // Attempt to allocate a very large size, expected to fail
    void *ptr = portable_malloc(0xFFFFFFFF);
    ASSERT(ptr == NULL, "portable_malloc should fail on overflow");
    printf("test_overflow PASSED\n");
    return 0;
}

int main() {
    int failures = 0;
    failures += test_basic_alloc_free();
    failures += test_multiple_allocs();
    failures += test_alignment();
    failures += test_overflow();

    if (failures == 0) {
        printf("ALL PORTABLE MALLOC TESTS PASSED\n");
        return 0;
    } else {
        printf("PORTABLE MALLOC TESTS FAILED: %d failures\n", failures);
        return 1;
    }
}
