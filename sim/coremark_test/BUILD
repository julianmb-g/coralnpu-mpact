load("@rules_shell//shell:sh_test.bzl", "sh_test")

genrule(
    name = "coremark_builder_image",
    srcs = ["coremark_builder.Dockerfile"],
    outs = ["coremark-builder.tar"],
    cmd = """
podman build -t coremark-builder:latest -f $(location coremark_builder.Dockerfile) .
podman save coremark-builder:latest > $@
""",
    tags = [
        "local",
        "requires-network",
    ],
    visibility = ["//sim/test:__pkg__"],
)

filegroup(
    name = "coremark_builder_image_tar",
    srcs = ["coremark-builder.tar"],
    visibility = ["//sim/test:__pkg__"],
)

genrule(
    name = "coremark_unified_asm",
    srcs = [
        ":coremark_builder_image",
        "build_unified_asm.sh",
        "common_cflags.sh",
        "//sim/test:testfiles/core_portme.h",
        "//sim/test:testfiles/core_portme.c",
        "format_asm.sh",
        "//sim/test:testfiles/crt0.S",
        "//sim/test:testfiles/linker.ld",
    ],
    outs = ["coremark_unified.S"],
    cmd = """
$(location //sim/test:testfiles/gen_coremark_asm.sh) \
  $(location //sim/test:testfiles/core_portme.h) \
  $(location //sim/test:testfiles/core_portme.c) \
  $(location build_unified_asm.sh) \
  $(location coremark_unified.S) \
  $(location common_cflags.sh) \
  $(location format_asm.sh) \
  $(location //sim/test:testfiles/crt0.S) \
  $(location //sim/test:testfiles/linker.ld)
""",
    tags = [
        "local",
        "requires-network",
    ],
    tools = ["//sim/test:testfiles/gen_coremark_asm.sh"],
)

genrule(
    name = "coremark_unified_elf",
    srcs = [
        ":coremark_builder_image",
        "build_unified_elf.sh",
        ":coremark_unified.S",
        "common_cflags.sh",
        "//sim/test:testfiles/linker.ld",
        "//sim/test:testfiles/crt0.S",
    ],
    outs = [
        "coremark_unified.elf",
        "coremark_unified.map",
        "coremark_unified.objdump",
    ],
    cmd = """
$(location //sim/test:testfiles/gen_coremark_elf.sh) \
  $(location //sim/test:testfiles/linker.ld) \
  $(location //sim/test:testfiles/crt0.S) \
  $(location :coremark_unified.S) \
  $(location build_unified_elf.sh) \
  $(location common_cflags.sh) \
  $(location coremark_unified.elf) \
  $(location coremark_unified.map) \
  $(location coremark_unified.objdump)
""",
    tags = ["local"],
    tools = ["//sim/test:testfiles/gen_coremark_elf.sh"],
)

sh_test(
    name = "coralnpu_m3_coremark_unified_asm_test",
    srcs = ["run_validation.sh"],
    args = [
        "$(location :coremark_unified.elf)",
        "$(location //sim:coralnpu_m3_sim)",
        "$(location :coremark_unified.objdump)",
    ],
    data = [
        "common_cflags.sh",
        "pty_runner.py",
        "run_validation.sh",
        "validate_output.sh",
        ":coremark_unified.elf",
        ":coremark_unified.objdump",
        "//sim:coralnpu_m3_sim",
    ],
    tags = ["local"],
)

sh_test(
    name = "coralnpu_m3_coremark_artifact_sync_test",
    srcs = ["diff_test.sh"],
    args = [
        "$(location :coremark_unified.S)",
        "$(location //sim/test:testfiles/coremark_unified.S)",
        "$(location :coremark_unified.map)",
        "$(location //sim/test:testfiles/coremark_unified.map)",
    ],
    data = [
        "common_cflags.sh",
        "diff_test.sh",
        "pty_runner.py",
        ":coremark_unified.S",
        ":coremark_unified.map",
        "//sim/test:testfiles/coremark_unified.S",
        "//sim/test:testfiles/coremark_unified.map",
    ],
    tags = ["local"],
)

sh_test(
    name = "coralnpu_m3_coremark_formatting_test",
    srcs = ["validate_formatting.sh"],
    args = [
        "$(location :coremark_unified.S)",
        "$(location //sim/test:testfiles/crt0.S)",
    ],
    data = [
        "common_cflags.sh",
        "pty_runner.py",
        "validate_formatting.sh",
        ":coremark_unified.S",
        "//sim/test:testfiles/crt0.S",
    ],
    tags = ["local"],
)

sh_test(
    name = "crt0_vector_test",
    srcs = ["crt0_vector_test.sh"],
    data = [
        "common_cflags.sh",
        "crt0_vector_test.sh",
        "pty_runner.py",
        ":coremark_unified.objdump",
    ],
    tags = ["local"],
)

sh_test(
    name = "coralnpu_m3_coremark_dynamic_asm_test",
    srcs = ["run_validation_dynamic.sh"],
    args = [
        "$(location //sim/test:testfiles/coremark_unified.S)",
        "$(location //sim/test:testfiles/crt0.S)",
        "$(location //sim/test:testfiles/linker.ld)",
        "$(location //sim:coralnpu_m3_sim)",
        "$(location :coremark_builder_image)",
    ],
    data = [
        "common_cflags.sh",
        "pty_runner.py",
        "run_validation_dynamic.sh",
        "validate_output.sh",
        ":coremark_builder_image",
        "//sim:coralnpu_m3_sim",
        "//sim/test:testfiles/core_portme.c",
        "//sim/test:testfiles/core_portme.h",
        "//sim/test:testfiles/coremark_unified.S",
        "//sim/test:testfiles/crt0.S",
        "//sim/test:testfiles/linker.ld",
    ],
    tags = [
        "local",
        "requires-network",
    ],
)

sh_test(
    name = "newlib_crt0_test",
    srcs = ["newlib_crt0_test.sh"],
    args = [
        "$(location //sim/test:testfiles/coremark_unified.S)",
        "$(location //sim/test:testfiles/linker.ld)",
        "$(location //sim:coralnpu_m3_sim)",
        "$(location :coremark_builder_image)",
    ],
    data = [
        "common_cflags.sh",
        "newlib_crt0_test.sh",
        "pty_runner.py",
        ":coremark_builder_image",
        "//sim:coralnpu_m3_sim",
        "//sim/test:testfiles/coremark_unified.S",
        "//sim/test:testfiles/linker.ld",
    ],
    tags = ["local"],
)

sh_test(
    name = "workspace_clean_test",
    srcs = ["workspace_clean_test.sh"],
    data = [
        "common_cflags.sh",
        "pty_runner.py",
        "workspace_clean_test.sh",
    ],
    tags = [
        "local",
    ],
)

sh_test(
    name = "test_ee_printf",
    srcs = ["test_ee_printf.sh"],
    args = [
        "$(location //sim:coralnpu_m3_sim)",
        "$(location :coremark_builder_image)",
    ],
    data = [
        "common_cflags.sh",
        "pty_runner.py",
        "test_ee_printf.c",
        "test_ee_printf.sh",
        ":coremark_builder_image",
        "//sim:coralnpu_m3_sim",
        "//sim/test:testfiles/core_portme.c",
        "//sim/test:testfiles/core_portme.h",
        "//sim/test:testfiles/crt0.S",
        "//sim/test:testfiles/linker.ld",
    ],
    tags = ["local"],
)

sh_test(
    name = "portable_malloc_test",
    srcs = ["portable_malloc_test.sh"],
    args = [
        "$(location //sim:coralnpu_m3_sim)",
        "$(location :coremark_builder_image)",
    ],
    data = [
        "common_cflags.sh",
        "portable_malloc_test.c",
        "portable_malloc_test.sh",
        "pty_runner.py",
        ":coremark_builder_image",
        "//sim:coralnpu_m3_sim",
        "//sim/test:testfiles/core_portme.c",
        "//sim/test:testfiles/core_portme.h",
        "//sim/test:testfiles/crt0.S",
        "//sim/test:testfiles/linker.ld",
    ],
    tags = ["local"],
)

sh_test(
    name = "coralnpu_m3_coremark_matrix_test",
    srcs = ["test_matrix.sh"],
    args = [
        "$(location :coremark_unified.objdump)",
    ],
    data = [
        "test_matrix.sh",
        ":coremark_unified.objdump",
    ],
    tags = ["local"],
)
