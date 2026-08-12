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
#include <stdint.h>
#include "core_portme.h"

int main() {
    ee_f32 x = 12.0f;
    ee_f32 y = 1.0f;

    FloatIntUnion u_x, u_y, u_expected;
    u_x.f = x;
    u_y.f = y;

    ee_f32 res1 = matrix_big(x);
    u_expected.i = 0xf000 | u_x.i;
    if (res1 != u_expected.f) {
        printf("Error: matrix_big failed\n");
        return 1;
    }

    ee_f32 res2 = matrix_clip(x, y);
    u_expected.i = u_y.i ? (u_x.i & 0x0ff) : (u_x.i & 0x0ffff);
    if (res2 != u_expected.f) {
        printf("Error: matrix_clip failed\n");
        return 1;
    }

    ee_f32 res3 = bit_extract(x, 4, 8);
    u_expected.i = (u_x.i >> 4) & 0xff;
    if (res3 != u_expected.f) {
        printf("Error: bit_extract failed\n");
        return 1;
    }

    printf("test_float_macros PASSED\n");
    return 0;
}
