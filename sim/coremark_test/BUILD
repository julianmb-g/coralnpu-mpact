# BUILD file for Coremark tests

load("@rules_shell//shell:sh_test.bzl", "sh_test")

package(default_visibility = ["//visibility:public"])

# Placeholder sh_test targets (will run compiled ELFs)
sh_test(
    name = "portable_malloc_test",
    srcs = ["run_portable_malloc_test.sh"],
    data = [
        ":portable_malloc_test.elf",
        ":portable_malloc_test.sh",
        "//sim:coralnpu_m3_sim",
    ],
    tags = ["local"],
)

sh_test(
    name = "crt0_mstatus_test",
    srcs = ["run_crt0_mstatus_test.sh"],
    data = [
        ":crt0_mstatus_test.elf",
        "//sim:coralnpu_m3_sim",
    ],
    tags = ["local"],
)

sh_test(
    name = "test_float_macros",
    srcs = ["run_test_float_macros.sh"],
    data = [
        ":test_float_macros.elf",
        "//sim:coralnpu_m3_sim",
    ],
    tags = ["local"],
)

sh_test(
    name = "crt0_vector_test",
    srcs = ["run_crt0_vector_test.sh"],
    data = [
        ":crt0_vector_test.elf",
        ":crt0_vector_test.sh",
        "//sim:coralnpu_m3_sim",
    ],
    tags = ["local"],
)

sh_test(
    name = "test_ee_printf",
    srcs = ["run_test_ee_printf.sh"],
    data = [
        ":test_ee_printf.elf",
        ":test_ee_printf.sh",
        "//sim:coralnpu_m3_sim",
    ],
    tags = ["local"],
)

sh_test(
    name = "newlib_crt0_test",
    srcs = ["run_newlib_crt0_test.sh"],
    data = [
        ":newlib_crt0_test.elf",
        ":newlib_crt0_test.sh",
        ":newlib_crt0_test_elf",  # This includes objdump and map
        "//sim:coralnpu_m3_sim",
    ],
    tags = ["local"],
)

# Genrule targets for test ELFs
genrule(
    name = "portable_malloc_test_elf",
    srcs = [
        "portable_malloc_test.c",
        "common_cflags.sh",
        "gen_test_elf.sh",
        "//sim/test:testfiles/core_portme.c",
        "//sim/test:testfiles/core_portme.h",
        "//sim/test:testfiles/crt0.S",
        "//sim/test:testfiles/linker.ld",
    ],
    outs = [
        "portable_malloc_test.elf",
        "portable_malloc_test.map",
        "portable_malloc_test.objdump",
    ],
    cmd = "$(location gen_test_elf.sh) $(location portable_malloc_test.c) $(location //sim/test:testfiles/linker.ld) $(location //sim/test:testfiles/crt0.S) $(location //sim/test:testfiles/core_portme.c) $(location //sim/test:testfiles/core_portme.h) $(location common_cflags.sh) $(location portable_malloc_test.elf) $(location portable_malloc_test.map) $(location portable_malloc_test.objdump)",
    local = 1,
    tags = [
        "local",
        "manual",
    ],
)

genrule(
    name = "crt0_mstatus_test_elf",
    srcs = [
        "crt0_mstatus_test.c",
        "common_cflags.sh",
        "gen_test_elf.sh",
        "//sim/test:testfiles/core_portme.c",
        "//sim/test:testfiles/core_portme.h",
        "//sim/test:testfiles/crt0.S",
        "//sim/test:testfiles/linker.ld",
    ],
    outs = [
        "crt0_mstatus_test.elf",
        "crt0_mstatus_test.map",
        "crt0_mstatus_test.objdump",
    ],
    cmd = "$(location gen_test_elf.sh) $(location crt0_mstatus_test.c) $(location //sim/test:testfiles/linker.ld) $(location //sim/test:testfiles/crt0.S) $(location //sim/test:testfiles/core_portme.c) $(location //sim/test:testfiles/core_portme.h) $(location common_cflags.sh) $(location crt0_mstatus_test.elf) $(location crt0_mstatus_test.map) $(location crt0_mstatus_test.objdump)",
    local = 1,
    tags = [
        "local",
        "manual",
    ],
)

genrule(
    name = "test_float_macros_elf",
    srcs = [
        "test_float_macros.c",
        "common_cflags.sh",
        "gen_test_elf.sh",
        "//sim/test:testfiles/core_portme.c",
        "//sim/test:testfiles/core_portme.h",
        "//sim/test:testfiles/crt0.S",
        "//sim/test:testfiles/linker.ld",
    ],
    outs = [
        "test_float_macros.elf",
        "test_float_macros.map",
        "test_float_macros.objdump",
    ],
    cmd = "$(location gen_test_elf.sh) $(location test_float_macros.c) $(location //sim/test:testfiles/linker.ld) $(location //sim/test:testfiles/crt0.S) $(location //sim/test:testfiles/core_portme.c) $(location //sim/test:testfiles/core_portme.h) $(location common_cflags.sh) $(location test_float_macros.elf) $(location test_float_macros.map) $(location test_float_macros.objdump)",
    local = 1,
    tags = [
        "local",
        "manual",
    ],
)

genrule(
    name = "crt0_vector_test_elf",
    srcs = [
        "crt0_vector_test.c",
        "common_cflags.sh",
        "gen_test_elf.sh",
        "//sim/test:testfiles/core_portme.c",
        "//sim/test:testfiles/core_portme.h",
        "//sim/test:testfiles/crt0.S",
        "//sim/test:testfiles/linker.ld",
    ],
    outs = [
        "crt0_vector_test.elf",
        "crt0_vector_test.map",
        "crt0_vector_test.objdump",
    ],
    cmd = "$(location gen_test_elf.sh) $(location crt0_vector_test.c) $(location //sim/test:testfiles/linker.ld) $(location //sim/test:testfiles/crt0.S) $(location //sim/test:testfiles/core_portme.c) $(location //sim/test:testfiles/core_portme.h) $(location common_cflags.sh) $(location crt0_vector_test.elf) $(location crt0_vector_test.map) $(location crt0_vector_test.objdump)",
    local = 1,
    tags = [
        "local",
        "manual",
    ],
)

genrule(
    name = "crt0_vector_test_v2_elf",
    srcs = [
        "crt0_vector_test.c",
        "common_cflags.sh",
        "gen_test_elf.sh",
        "//sim/test:testfiles/core_portme.c",
        "//sim/test:testfiles/core_portme.h",
        "//sim/test:testfiles/crt0.S",
        "//sim/test:testfiles/linker.ld",
    ],
    outs = [
        "crt0_vector_test_v2.elf",
        "crt0_vector_test_v2.map",
        "crt0_vector_test_v2.objdump",
    ],
    cmd = "$(location gen_test_elf.sh) $(location crt0_vector_test.c) $(location //sim/test:testfiles/linker.ld) $(location //sim/test:testfiles/crt0.S) $(location //sim/test:testfiles/core_portme.c) $(location //sim/test:testfiles/core_portme.h) $(location common_cflags.sh) $(location crt0_vector_test_v2.elf) $(location crt0_vector_test_v2.map) $(location crt0_vector_test_v2.objdump) -march=rv32imf_zve32x -DENABLE_V2",
    local = 1,
    tags = [
        "local",
        "manual",
    ],
)

sh_test(
    name = "run_v2_vector_test",
    srcs = ["crt0_vector_test.sh"],
    args = [
        "$(location //sim:coralnpu_v2_sim)",
        "$(location :crt0_vector_test_v2.elf)",
    ],
    data = [
        ":crt0_vector_test.sh",
        ":crt0_vector_test_v2.elf",
        "//sim:coralnpu_v2_sim",
    ],
    tags = ["local"],
)

genrule(
    name = "crt0_vector_test_m4_elf",
    srcs = [
        "crt0_vector_test.c",
        "common_cflags.sh",
        "gen_test_elf.sh",
        "//sim/test:testfiles/core_portme.c",
        "//sim/test:testfiles/core_portme.h",
        "//sim/test:testfiles/crt0.S",
        "//sim/test:testfiles/linker.ld",
    ],
    outs = [
        "crt0_vector_test_m4.elf",
        "crt0_vector_test_m4.map",
        "crt0_vector_test_m4.objdump",
    ],
    cmd = "$(location gen_test_elf.sh) $(location crt0_vector_test.c) $(location //sim/test:testfiles/linker.ld) $(location //sim/test:testfiles/crt0.S) $(location //sim/test:testfiles/core_portme.c) $(location //sim/test:testfiles/core_portme.h) $(location common_cflags.sh) $(location crt0_vector_test_m4.elf) $(location crt0_vector_test_m4.map) $(location crt0_vector_test_m4.objdump) -DENABLE_M4",
    local = 1,
    tags = [
        "local",
        "manual",
    ],
)

genrule(
    name = "test_ee_printf_elf",
    srcs = [
        "test_ee_printf.c",
        "common_cflags.sh",
        "gen_test_elf.sh",
        "//sim/test:testfiles/core_portme.c",
        "//sim/test:testfiles/core_portme.h",
        "//sim/test:testfiles/crt0.S",
        "//sim/test:testfiles/linker.ld",
    ],
    outs = [
        "test_ee_printf.elf",
        "test_ee_printf.map",
        "test_ee_printf.objdump",
    ],
    cmd = "$(location gen_test_elf.sh) $(location test_ee_printf.c) $(location //sim/test:testfiles/linker.ld) $(location //sim/test:testfiles/crt0.S) $(location //sim/test:testfiles/core_portme.c) $(location //sim/test:testfiles/core_portme.h) $(location common_cflags.sh) $(location test_ee_printf.elf) $(location test_ee_printf.map) $(location test_ee_printf.objdump)",
    local = 1,
    tags = [
        "local",
        "manual",
    ],
)

genrule(
    name = "newlib_crt0_test_elf",
    srcs = [
        "newlib_crt0_test.c",
        "common_cflags.sh",
        "gen_test_elf.sh",
        "//sim/test:testfiles/core_portme.c",
        "//sim/test:testfiles/core_portme.h",
        "//sim/test:testfiles/crt0.S",
        "//sim/test:testfiles/linker.ld",
    ],
    outs = [
        "newlib_crt0_test.elf",
        "newlib_crt0_test.map",
        "newlib_crt0_test.objdump",
    ],
    cmd = "$(location gen_test_elf.sh) $(location newlib_crt0_test.c) $(location //sim/test:testfiles/linker.ld) $(location //sim/test:testfiles/crt0.S) $(location //sim/test:testfiles/core_portme.c) $(location //sim/test:testfiles/core_portme.h) $(location common_cflags.sh) $(location newlib_crt0_test.elf) $(location newlib_crt0_test.map) $(location newlib_crt0_test.objdump)",
    local = 1,
    tags = [
        "local",
        "manual",
    ],
)

sh_test(
    name = "run_final_test",
    timeout = "long",
    srcs = ["run_final_test.sh"],
    args = ["$(location //sim:coralnpu_m3_sim)"],
    data = [
        ":common_cflags.sh",
        ":coremark_unified_artifacts",
        ":validate_output.sh",
        "//sim:coralnpu_m3_sim",
        "//sim:coralnpu_m4_sim",
        "//sim:coralnpu_v2_sim",
    ],
    tags = ["local"],
)

sh_test(
    name = "run_m4_test",
    timeout = "long",
    srcs = ["run_final_test.sh"],
    args = ["$(location //sim:coralnpu_m4_sim)"],
    data = [
        ":common_cflags.sh",
        ":coremark_unified_artifacts",
        ":validate_output.sh",
        "//sim:coralnpu_m4_sim",
    ],
    tags = ["local"],
)

sh_test(
    name = "run_m4_vector_test",
    srcs = ["crt0_vector_test.sh"],
    args = [
        "$(location //sim:coralnpu_m4_sim)",
        "$(location :crt0_vector_test_m4.elf)",
    ],
    data = [
        ":crt0_vector_test.sh",
        ":crt0_vector_test_m4.elf",
        "//sim:coralnpu_m4_sim",
    ],
    tags = ["local"],
)

# 4.2.3: Verify artifact placement
sh_test(
    name = "verify_artifacts",
    srcs = ["verify_artifacts.sh"],
    data = [
        ":coremark_unified_artifacts",  # Depends on artifacts being generated
        ":scan_asm.sh",
        ":update.sh",
        "//sim/test:testfiles/core_portme.c",
        "//sim/test:testfiles/core_portme.h",
        "//sim/test:testfiles/crt0.S",
        "//sim/test:testfiles/linker.ld",
    ],
    tags = ["local"],
)

# 3.5.2: Genrule to build unified assembly and map file
genrule(
    name = "coremark_unified_artifacts",
    srcs = [
        "build_unified_asm.sh",
        "common_cflags.sh",
        "coremark_unified.c",
        "//sim/test:testfiles/core_portme.c",
        "//sim/test:testfiles/core_portme.h",
        "//sim/test:testfiles/crt0.S",
        "//sim/test:testfiles/linker.ld",
        ":format_asm.sh",
    ],
    outs = [
        "coremark_unified.S",
        "coremark_unified.map",
        "coremark_unified.elf",
        "coremark_unified.objdump",
    ],
    cmd = "$(location :build_unified_asm.sh) $(@D) $(location :common_cflags.sh) $(location :coremark_unified.c) $(location //sim/test:testfiles/core_portme.c) $(location //sim/test:testfiles/core_portme.h) $(location //sim/test:testfiles/crt0.S) $(location //sim/test:testfiles/linker.ld) $(location :format_asm.sh)",
    local = 1,
    tags = [
        "local",
        "manual",
    ],  # Prevent inclusion in wildcards
)

filegroup(
    name = "all_sh_files",
    srcs = glob(["*.sh"]),
)

sh_test(
    name = "validate_formatting",
    srcs = ["validate_formatting.sh"],
    args = ["$(locations :all_sh_files)"],
    data = [":all_sh_files"],
    tags = ["local"],
)

sh_test(
    name = "validate_build_tags",
    srcs = ["validate_build_tags.sh"],
    data = [
        ":BUILD",
        ":build_unified_asm.sh",
        ":gen_coremark_elf.sh",
        "//sim/test:testfiles/core_portme.c",
        "//sim/test:testfiles/core_portme.h",
        "//sim/test:testfiles/crt0.S",
    ],
    tags = ["local"],
)
