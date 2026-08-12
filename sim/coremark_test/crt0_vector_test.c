// Copyright 2026 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <stdint.h>

// Simple assertion macro
#define ASSERT(cond, msg) \
    if (!(cond)) { \
        printf("ASSERT FAILED: %s in %s at line %d\n", msg, __FILE__, __LINE__); \
        return 1; \
    }

int main() {
    uint32_t vtype_val, vl_val, vstart_val, fcsr_val, fflags_val, frm_val;
#if defined(__riscv_zve32f)
    uint32_t vxsat_val, vxrm_val, vlenb_val, vcsr_val;
#endif

    // Read Vector CSRs
    __asm__ volatile ("csrr %0, vtype" : "=r"(vtype_val));
    __asm__ volatile ("csrr %0, vl" : "=r"(vl_val));
    __asm__ volatile ("csrr %0, vstart" : "=r"(vstart_val));
#if defined(__riscv_zve32f)
    __asm__ volatile ("csrr %0, vxsat" : "=r"(vxsat_val));
    __asm__ volatile ("csrr %0, vxrm" : "=r"(vxrm_val));
    __asm__ volatile ("csrr %0, vlenb" : "=r"(vlenb_val));
    __asm__ volatile ("csrr %0, vcsr" : "=r"(vcsr_val));
#endif
    __asm__ volatile ("csrr %0, fcsr" : "=r"(fcsr_val));
    __asm__ volatile ("csrr %0, fflags" : "=r"(fflags_val));
    __asm__ volatile ("csrr %0, frm" : "=r"(frm_val));

#if defined(__riscv_zve32f)
    printf("vtype: 0x%x, vl: %d, vstart: %d, vxsat: %d, vxrm: %d, vlenb: %d, vcsr: 0x%x, fcsr: 0x%x, fflags: 0x%x, frm: 0x%x\n", 
           vtype_val, vl_val, vstart_val, vxsat_val, vxrm_val, vlenb_val, vcsr_val, fcsr_val, fflags_val, frm_val);
#else
    printf("vtype: 0x%x, vl: %d, vstart: %d, fcsr: 0x%x, fflags: 0x%x, frm: 0x%x\n", 
           vtype_val, vl_val, vstart_val, fcsr_val, fflags_val, frm_val);
#endif

    // vtype and vl should be non-zero after vsetvli in crt0
    ASSERT(vtype_val != 0, "vtype not initialized (is zero)");
    ASSERT(vl_val != 0, "vl not initialized (is zero)");

    // vstart should be zeroed
    ASSERT(vstart_val == 0, "vstart not initialized to zero");

#if defined(__riscv_zve32f)
    // vxsat, vxrm, and vcsr should be zeroed
    ASSERT(vxsat_val == 0, "vxsat not initialized to zero");
    ASSERT(vxrm_val == 0, "vxrm not initialized to zero");
    ASSERT(vcsr_val == 0, "vcsr not initialized to zero");

    // Exact assertions based on vlenb for vtype and vl (assuming e32, m1)
    // vl should be VLEN/32 = (vlenb * 8) / 32 = vlenb / 4
    ASSERT(vl_val == (vlenb_val / 4), "vl does not match expected value for e32, m1");
#endif

    // vtype for e32, m1, ta, ma is typically 0xD0 or 0xD2 (depends on exact bits)
    // We'll assert vtype is at least > 0
    ASSERT(vtype_val == 0xd0, "vtype does not match expected value for e32, m1, ta, ma");

    // fcsr should be zeroed
    ASSERT(fcsr_val == 0, "fcsr not initialized to zero");
    ASSERT(fflags_val == 0, "fflags not initialized to zero");
    ASSERT(frm_val == 0, "frm not initialized to zero");

    // Read first element of vector registers
    uint32_t v0_val, v15_val, v16_val, v31_val;
    __asm__ volatile ("vmv.x.s %0, v0" : "=r"(v0_val));
    __asm__ volatile ("vmv.x.s %0, v15" : "=r"(v15_val));
    __asm__ volatile ("vmv.x.s %0, v16" : "=r"(v16_val));
    __asm__ volatile ("vmv.x.s %0, v31" : "=r"(v31_val));

    ASSERT(v0_val == 0, "v0 not initialized to zero");
    ASSERT(v15_val == 0, "v15 not initialized to zero");
    ASSERT(v16_val == 0, "v16 not initialized to zero");
    ASSERT(v31_val == 0, "v31 not initialized to zero");

    // Read scalar FP registers
    uint32_t f0_val, f15_val, f16_val, f31_val;
    __asm__ volatile ("fmv.x.w %0, f0" : "=r"(f0_val));
    __asm__ volatile ("fmv.x.w %0, f15" : "=r"(f15_val));
    __asm__ volatile ("fmv.x.w %0, f16" : "=r"(f16_val));
    __asm__ volatile ("fmv.x.w %0, f31" : "=r"(f31_val));

    ASSERT(f0_val == 0, "f0 not initialized to zero");
    ASSERT(f15_val == 0, "f15 not initialized to zero");
    ASSERT(f16_val == 0, "f16 not initialized to zero");
    ASSERT(f31_val == 0, "f31 not initialized to zero");

#if defined(__riscv_zve32f)
    // vlenb should be consistent with the hardware (e.g., 4 for VLEN=32)
    // For zve32f, VLEN is at least 32.
    ASSERT(vlenb_val >= 4, "vlenb is less than 4 bytes (VLEN < 32)");
#endif

#if defined(ENABLE_M4)
    uint32_t mtype_val;
    __asm__ volatile ("csrr %0, 0xC23" : "=r"(mtype_val));
    ASSERT(mtype_val == 0x843, "mtype not initialized correctly to 0x843");
#endif

    printf("crt0_vector_test PASSED\n");
    return 0;
}
