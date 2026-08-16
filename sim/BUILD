# Copyright 2023 Google LLC
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

# Build rules for CoralNPU simulator code.
load("@mpact-sim//mpact/sim/decoder:mpact_sim_isa.bzl", "mpact_bin_fmt_decoder", "mpact_isa_decoder")
load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_cc//cc:defs.bzl", "cc_test")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "coralnpu_state",
    srcs = [
        "coralnpu_state.cc",
    ],
    hdrs = [
        "coralnpu_state.h",
    ],
    copts = [
        "-O3",
        "-ffp-model=strict",
        "-fprotect-parens",
    ],
    deps = [
        "@abseil-cpp//absl/functional:any_invocable",
        "@abseil-cpp//absl/log",
        "@abseil-cpp//absl/log:check",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/strings:str_format",
        "@mpact-riscv//riscv:riscv_state",
        "@mpact-sim//mpact/sim/generic:instruction",
        "@mpact-sim//mpact/sim/util/memory",
    ],
)

cc_library(
    name = "coralnpu_instructions",
    srcs = [
        "coralnpu_instructions.cc",
        "coralnpu_vector_convolution_instructions.cc",
        "coralnpu_vector_instructions.cc",
        "coralnpu_vector_memory_instructions.cc",
    ],
    hdrs = [
        "coralnpu_instructions.h",
        "coralnpu_vector_convolution_instructions.h",
        "coralnpu_vector_instructions.h",
        "coralnpu_vector_memory_instructions.h",
    ],
    copts = ["-O3"],
    deps = [
        ":coralnpu_state",
        "@abseil-cpp//absl/functional:bind_front",
        "@abseil-cpp//absl/log:check",
        "@abseil-cpp//absl/numeric:bits",
        "@abseil-cpp//absl/types:span",
        "@mpact-riscv//riscv:riscv_state",
        "@mpact-riscv//riscv:riscv_fp_state",
        "@mpact-sim//mpact/sim/generic:arch_state",
        "@mpact-sim//mpact/sim/generic:core",
        "@mpact-sim//mpact/sim/generic:instruction",
        "@mpact-sim//mpact/sim/generic:type_helpers",
    ],
)

mpact_isa_decoder(
    name = "coralnpu_isa",
    src = "coralnpu.isa",
    includes = [
        "coralnpu_arith.isa",
        "coralnpu_base.isa",
        "coralnpu_memory.isa",
        "coralnpu_mul.isa",
        "coralnpu_shift.isa",
        "@mpact-riscv//riscv:riscv32zb.isa",
    ],
    isa_name = "CoralNPU",
    deps = [
        ":coralnpu_instructions",
        "@abseil-cpp//absl/functional:bind_front",
        "@mpact-riscv//riscv:riscv_bitmanip_instructions",
        "@mpact-riscv//riscv:riscv_g",
    ],
)

mpact_bin_fmt_decoder(
    name = "coralnpu_bin_fmt",
    src = "coralnpu.bin_fmt",
    decoder_name = "CoralNPU",
    includes = [
        "coralnpu_arith.bin_fmt",
        "coralnpu_base.bin_fmt",
        "coralnpu_conv.bin_fmt",
        "coralnpu_format.bin_fmt",
        "coralnpu_memory.bin_fmt",
        "coralnpu_mul.bin_fmt",
        "coralnpu_shift.bin_fmt",
        "@mpact-riscv//riscv:riscv32zb.bin_fmt",
    ],
    deps = [
        ":coralnpu_isa",
    ],
)

cc_library(
    name = "coralnpu_decoder",
    srcs = [
        "coralnpu_encoding.cc",
        "decoder.cc",
    ],
    hdrs = [
        "coralnpu_encoding.h",
        "decoder.h",
    ],
    copts = ["-O3"],
    deps = [
        ":coralnpu_bin_fmt",
        ":coralnpu_isa",
        ":coralnpu_state",
        "@abseil-cpp//absl/container:flat_hash_map",
        "@abseil-cpp//absl/functional:any_invocable",
        "@abseil-cpp//absl/log",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/types:span",
        "@mpact-riscv//riscv:riscv_generic_decoder",
        "@mpact-riscv//riscv:riscv_state",
        "@mpact-sim//mpact/sim/generic:arch_state",
        "@mpact-sim//mpact/sim/generic:core",
        "@mpact-sim//mpact/sim/generic:instruction",
        "@mpact-sim//mpact/sim/generic:program_error",
        "@mpact-sim//mpact/sim/generic:type_helpers",
        "@mpact-sim//mpact/sim/util/memory",
    ],
)

cc_library(
    name = "coralnpu_top",
    srcs = [
        "coralnpu_action_point_memory_interface.cc",
        "coralnpu_top.cc",
    ],
    hdrs = [
        "coralnpu_action_point_memory_interface.h",
        "coralnpu_top.h",
    ],
    copts = ["-O3"],
    deps = [
        ":coralnpu_decoder",
        ":coralnpu_isa",
        ":coralnpu_state",
        "//sim/proto:trace_cc_proto",
        "//sim/renode:coralnpu_renode_memory",
        "@abseil-cpp//absl/container:flat_hash_map",
        "@abseil-cpp//absl/flags:flag",
        "@abseil-cpp//absl/functional:any_invocable",
        "@abseil-cpp//absl/functional:bind_front",
        "@abseil-cpp//absl/log",
        "@abseil-cpp//absl/log:check",
        "@abseil-cpp//absl/status",
        "@abseil-cpp//absl/status:statusor",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/synchronization",
        "@mpact-riscv//riscv:riscv_arm_semihost",
        "@mpact-riscv//riscv:riscv_fp_state",
        "@mpact-riscv//riscv:riscv_state",
        "@mpact-sim//mpact/sim/generic:action_points",
        "@mpact-sim//mpact/sim/generic:arch_state",
        "@mpact-sim//mpact/sim/generic:component",
        "@mpact-sim//mpact/sim/generic:core",
        "@mpact-sim//mpact/sim/generic:core_debug_interface",
        "@mpact-sim//mpact/sim/generic:counters",
        "@mpact-sim//mpact/sim/generic:decode_cache",
        "@mpact-sim//mpact/sim/generic:type_helpers",
        "@mpact-sim//mpact/sim/util/memory",
    ],
)

cc_binary(
    name = "coralnpu_sim",
    srcs = ["coralnpu_sim.cc"],
    copts = ["-O3"],
    data = [
        "//sim/test:testfiles/hello_world_rv32imf.elf",
        "//sim/test:testfiles/rv32i.elf",
        "//sim/test:testfiles/rv32m.elf",
        "//sim/test:testfiles/rv32soft_fp.elf",
    ],
    deps = [
        ":coralnpu_state",
        ":coralnpu_top",
        "@abseil-cpp//absl/flags:flag",
        "@abseil-cpp//absl/flags:parse",
        "@abseil-cpp//absl/flags:usage",
        "@abseil-cpp//absl/log",
        "@abseil-cpp//absl/log:initialize",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/strings:str_format",
        "@abseil-cpp//absl/time",
        "@com_googlesource_code_re2//:re2",
        "@mpact-riscv//riscv:debug_command_shell",
        "@mpact-riscv//riscv:riscv_state",
        "@mpact-sim//mpact/sim/util/program_loader:elf_loader",
    ],
)

cc_library(
    name = "coralnpu_v2_state",
    srcs = [
        "coralnpu_v2_state.cc",
    ],
    hdrs = [
        "coralnpu_v2_state.h",
    ],
    copts = [
        "-O3",
    ],
    deps = [
        "@abseil-cpp//absl/functional:any_invocable",
        "@mpact-riscv//riscv:riscv_state",
        "@mpact-sim//mpact/sim/generic:instruction",
        "@mpact-sim//mpact/sim/util/memory",
    ],
)

mpact_isa_decoder(
    name = "coralnpu_v2_isa",
    src = "coralnpu_v2.isa",
    includes = [
        "@mpact-riscv//riscv:riscv32g.isa",
        "@mpact-riscv//riscv:riscv32zb.isa",
        "@mpact-riscv//riscv:riscv_vector.isa",
    ],
    isa_name = "CoralNPUV2",
    deps = [
        ":coralnpu_v2_instructions",
        "@abseil-cpp//absl/functional:bind_front",
        "@mpact-riscv//riscv:riscv_bitmanip_instructions",
        "@mpact-riscv//riscv:riscv_g",
        "@mpact-riscv//riscv:riscv_v",
    ],
)

mpact_bin_fmt_decoder(
    name = "coralnpu_v2_bin_fmt",
    src = "coralnpu_v2.bin_fmt",
    decoder_name = "CoralNPUV2",
    includes = [
        "@mpact-riscv//riscv:riscv32g.bin_fmt",
        "@mpact-riscv//riscv:riscv_format16.bin_fmt",
        "@mpact-riscv//riscv:riscv_format32.bin_fmt",
        "@mpact-riscv//riscv:riscv_vector.bin_fmt",
    ],
    deps = [
        ":coralnpu_v2_isa",
    ],
)

cc_library(
    name = "coralnpu_v2_encoding_template",
    hdrs = ["coralnpu_v2_encoding_template.h"],
    deps = [
        ":coralnpu_v2_getters",
        ":coralnpu_v2_state",
        "@abseil-cpp//absl/container:flat_hash_map",
        "@abseil-cpp//absl/functional:any_invocable",
        "@mpact-riscv//riscv:riscv_encoding_common",
        "@mpact-sim//mpact/sim/generic:core",
    ],
)

cc_library(
    name = "coralnpu_v2_user_decoder",
    srcs = [
        "coralnpu_v2_encoding.cc",
        "coralnpu_v2_user_decoder.cc",
    ],
    hdrs = [
        "coralnpu_v2_encoding.h",
        "coralnpu_v2_user_decoder.h",
    ],
    copts = [
        "-O3",
    ],
    deps = [
        ":coralnpu_v2_bin_fmt",
        ":coralnpu_v2_encoding_template",
        ":coralnpu_v2_isa",
        ":coralnpu_v2_state",
        "@mpact-riscv//riscv:riscv_generic_decoder",
        "@mpact-riscv//riscv:riscv_state",
        "@mpact-sim//mpact/sim/generic:arch_state",
        "@mpact-sim//mpact/sim/generic:core",
        "@mpact-sim//mpact/sim/generic:instruction",
        "@mpact-sim//mpact/sim/generic:program_error",
        "@mpact-sim//mpact/sim/generic:type_helpers",
        "@mpact-sim//mpact/sim/util/memory",
    ],
)

cc_library(
    name = "coralnpu_v2_getters",
    hdrs = ["coralnpu_v2_getters.h"],
    copts = ["-O3"],
    deps = [
        "@abseil-cpp//absl/container:flat_hash_map",
        "@abseil-cpp//absl/functional:any_invocable",
        "@abseil-cpp//absl/status:statusor",
        "@abseil-cpp//absl/strings",
        "@mpact-riscv//riscv:riscv_encoding_common",
        "@mpact-riscv//riscv:riscv_getters",
        "@mpact-riscv//riscv:riscv_state",
        "@mpact-sim//mpact/sim/generic:core",
        "@mpact-sim//mpact/sim/generic:type_helpers",
    ],
)

cc_library(
    name = "coralnpu_v2_instructions",
    srcs = [
        "coralnpu_v2_instructions.cc",
    ],
    hdrs = [
        "coralnpu_v2_instructions.h",
    ],
    copts = ["-O3"],
    deps = [
        ":coralnpu_v2_state",
        "@abseil-cpp//absl/types:span",
        "@mpact-riscv//riscv:riscv_g",
        "@mpact-riscv//riscv:riscv_state",
        "@mpact-riscv//riscv:riscv_v",
        "@mpact-sim//mpact/sim/generic:core",
        "@mpact-sim//mpact/sim/generic:instruction",
        "@mpact-sim//mpact/sim/generic:type_helpers",
    ],
)

cc_library(
    name = "coralnpu_architecture",
    hdrs = ["coralnpu_architecture.h"],
)

cc_library(
    name = "coralnpu_simulator",
    srcs = ["coralnpu_simulator.cc"],
    hdrs = ["coralnpu_simulator.h"],
    copts = ["-O3"],
    deps = [
        ":coralnpu_architecture",
        ":coralnpu_m3_user_decoder",
        ":coralnpu_m4_user_decoder",
        ":coralnpu_v2_state",
        ":coralnpu_v2_user_decoder",
        "@abseil-cpp//absl/log",
        "@abseil-cpp//absl/log:check",
        "@abseil-cpp//absl/status",
        "@abseil-cpp//absl/status:statusor",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/strings:str_format",
        "@com_github_serge1_elfio//:elfio",
        "@mpact-riscv//riscv:debug_command_shell",
        "@mpact-riscv//riscv:riscv32_htif_semihost",
        "@mpact-riscv//riscv:riscv_fp_state",
        "@mpact-riscv//riscv:riscv_state",
        "@mpact-riscv//riscv:riscv_top",
        "@mpact-riscv//riscv:riscv_zvt_state",
        "@mpact-sim//mpact/sim/generic:core",
        "@mpact-sim//mpact/sim/generic:core_debug_interface",
        "@mpact-sim//mpact/sim/generic:instruction",
        "@mpact-sim//mpact/sim/generic:type_helpers",
        "@mpact-sim//mpact/sim/util/memory",
        "@mpact-sim//mpact/sim/util/program_loader:elf_loader",
    ],
)

cc_binary(
    name = "coralnpu_v2_sim",
    srcs = ["coralnpu_v2_sim.cc"],
    copts = ["-O3"],
    deps = [
        ":coralnpu_simulator",
        ":coralnpu_v2_state",
        "@abseil-cpp//absl/flags:flag",
        "@abseil-cpp//absl/flags:marshalling",
        "@abseil-cpp//absl/flags:parse",
        "@abseil-cpp//absl/flags:usage",
        "@abseil-cpp//absl/log",
        "@abseil-cpp//absl/log:initialize",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/strings:str_format",
        "@abseil-cpp//absl/strings:string_view",
        "@abseil-cpp//absl/time",
        "@mpact-sim//mpact/sim/generic:type_helpers",
    ],
)

cc_library(
    name = "coralnpu_m3_zfhmin_instructions",
    srcs = ["coralnpu_m3_zfhmin_instructions.cc"],
    hdrs = ["coralnpu_m3_zfhmin_instructions.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        ":coralnpu_state",
        ":coralnpu_v2_state",
        "@mpact-riscv//riscv:riscv_state",
        "@mpact-sim//mpact/sim/generic:instruction",
    ],
)

cc_library(
    name = "coralnpu_m3_zfbfmin_overrides",
    srcs = ["coralnpu_m3_zfbfmin_overrides.cc"],
    hdrs = ["coralnpu_m3_zfbfmin_overrides.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        ":coralnpu_v2_state",
        "@abseil-cpp//absl/base",
        "@abseil-cpp//absl/strings",
        "@mpact-riscv//riscv:riscv_fp_state",
        "@mpact-riscv//riscv:riscv_g",
        "@mpact-riscv//riscv:riscv_state",
        "@mpact-sim//mpact/sim/generic:instruction",
        "@mpact-sim//mpact/sim/generic:type_helpers",
    ],
)

mpact_isa_decoder(
    name = "coralnpu_m3_isa",
    src = "coralnpu_m3.isa",
    includes = [
        "coralnpu_v2.isa",
        "@mpact-riscv//riscv:riscv_zvt.isa",
    ],
    isa_name = "CoralNPUM3",
    deps = [
        ":coralnpu_instructions",
        ":coralnpu_m3_zfbfmin_overrides",
        ":coralnpu_m3_zfhmin_instructions",
        ":coralnpu_v2_instructions",
        ":coralnpu_v2_isa",
        "@abseil-cpp//absl/functional:bind_front",
        "@mpact-riscv//riscv:riscv_bitmanip_instructions",
        "@mpact-riscv//riscv:riscv_g",
        "@mpact-riscv//riscv:riscv_v",
        "@mpact-riscv//riscv:riscv_zfh_instructions",
    ],
)

mpact_bin_fmt_decoder(
    name = "coralnpu_m3_bin_fmt",
    src = "coralnpu_m3.bin_fmt",
    decoder_name = "CoralNPUM3",
    includes = [
        "coralnpu_m3_decoder.h",
        "coralnpu_v2.bin_fmt",
        "@mpact-riscv//riscv:riscv_zvt.bin_fmt",
    ],
    deps = [
        ":coralnpu_m3_isa",
        ":coralnpu_v2_bin_fmt",
    ],
)

cc_library(
    name = "coralnpu_m3_user_decoder",
    srcs = [
        "coralnpu_m3_encoding.cc",
        "coralnpu_m3_user_decoder.cc",
    ],
    hdrs = [
        "coralnpu_m3_encoding.h",
        "coralnpu_m3_user_decoder.h",
    ],
    copts = ["-O3"],
    deps = [
        ":coralnpu_m3_bin_fmt",
        ":coralnpu_m3_isa",
        ":coralnpu_v2_encoding_template",
        ":coralnpu_v2_state",
        "@mpact-riscv//riscv:riscv_generic_decoder",
        "@mpact-riscv//riscv:riscv_state",
        "@mpact-sim//mpact/sim/generic:arch_state",
        "@mpact-sim//mpact/sim/generic:core",
        "@mpact-sim//mpact/sim/generic:instruction",
        "@mpact-sim//mpact/sim/generic:program_error",
        "@mpact-sim//mpact/sim/util/memory",
    ],
)

cc_library(
    name = "coralnpu_m3_assembler_lib",
    srcs = [
        "coralnpu_m3_assembler.cc",
        "coralnpu_m3_bin_encoder_interface.cc",
    ],
    hdrs = [
        "coralnpu_m3_assembler.h",
        "coralnpu_m3_bin_encoder_interface.h",
    ],
    deps = [
        ":coralnpu_m3_bin_fmt",
        ":coralnpu_m3_isa",
        "@abseil-cpp//absl/base:no_destructor",
        "@abseil-cpp//absl/container:flat_hash_map",
        "@abseil-cpp//absl/log:check",
        "@abseil-cpp//absl/status",
        "@abseil-cpp//absl/status:statusor",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/strings:str_format",
        "@com_googlesource_code_re2//:re2",
        "@mpact-sim//mpact/sim/util/asm",
    ],
)

cc_binary(
    name = "coralnpu_m3_as",
    srcs = ["coralnpu_m3_as_main.cc"],
    deps = [
        ":coralnpu_m3_assembler_lib",
        ":coralnpu_m3_isa",
        "@abseil-cpp//absl/flags:flag",
        "@abseil-cpp//absl/flags:parse",
        "@abseil-cpp//absl/log",
        "@abseil-cpp//absl/log:check",
        "@abseil-cpp//absl/log:initialize",
        "@abseil-cpp//absl/status",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/strings:str_format",
        "@com_github_serge1_elfio//:elfio",
        "@mpact-sim//mpact/sim/util/asm:simple_assembler",
    ],
)

cc_binary(
    name = "coralnpu_m3_sim",
    srcs = ["coralnpu_m3_sim.cc"],
    copts = ["-O3"],
    deps = [
        ":coralnpu_architecture",
        ":coralnpu_simulator",
        ":coralnpu_v2_state",
        "@abseil-cpp//absl/flags:flag",
        "@abseil-cpp//absl/flags:marshalling",
        "@abseil-cpp//absl/flags:parse",
        "@abseil-cpp//absl/flags:usage",
        "@abseil-cpp//absl/log",
        "@abseil-cpp//absl/log:initialize",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/strings:str_format",
        "@abseil-cpp//absl/strings:string_view",
        "@abseil-cpp//absl/time",
        "@mpact-sim//mpact/sim/generic:type_helpers",
    ],
)

mpact_isa_decoder(
    name = "coralnpu_m4_isa",
    src = "coralnpu_m4.isa",
    includes = [
        "coralnpu_m3.isa",
        "coralnpu_m4_instructions.h",
        "coralnpu_v2.isa",
        "@mpact-riscv//riscv:riscv_zvt.isa",
    ],
    isa_name = "CoralNPUM4",
    deps = [
        ":coralnpu_m3_isa",
        ":coralnpu_m4_instructions",
        ":coralnpu_v2_instructions",
        ":coralnpu_v2_isa",
        "@abseil-cpp//absl/functional:bind_front",
        "@mpact-riscv//riscv:riscv_bitmanip_instructions",
        "@mpact-riscv//riscv:riscv_g",
        "@mpact-riscv//riscv:riscv_v",
        "@mpact-riscv//riscv:riscv_zvt_instructions",
    ],
)

mpact_bin_fmt_decoder(
    name = "coralnpu_m4_bin_fmt",
    src = "coralnpu_m4.bin_fmt",
    decoder_name = "CoralNPUM4",
    includes = [
        "coralnpu_m3.bin_fmt",
        "coralnpu_m4_decoder.h",
        "coralnpu_v2.bin_fmt",
        "@mpact-riscv//riscv:riscv_zvt.bin_fmt",
    ],
    deps = [
        ":coralnpu_m3_bin_fmt",
        ":coralnpu_m4_isa",
        ":coralnpu_v2_bin_fmt",
    ],
)

cc_library(
    name = "coralnpu_m4_user_decoder",
    srcs = [
        "coralnpu_m4_encoding.cc",
        "coralnpu_m4_user_decoder.cc",
    ],
    hdrs = [
        "coralnpu_m4_encoding.h",
        "coralnpu_m4_user_decoder.h",
    ],
    copts = ["-O3"],
    deps = [
        ":coralnpu_m4_bin_fmt",
        ":coralnpu_m4_isa",
        ":coralnpu_v2_encoding_template",
        ":coralnpu_v2_state",
        "@mpact-riscv//riscv:riscv_generic_decoder",
        "@mpact-riscv//riscv:riscv_state",
        "@mpact-riscv//riscv:riscv_zvt_getters",
        "@mpact-sim//mpact/sim/generic:arch_state",
        "@mpact-sim//mpact/sim/generic:core",
        "@mpact-sim//mpact/sim/generic:instruction",
        "@mpact-sim//mpact/sim/generic:program_error",
        "@mpact-sim//mpact/sim/util/memory",
    ],
)

cc_binary(
    name = "coralnpu_m4_sim",
    srcs = ["coralnpu_m4_sim.cc"],
    copts = ["-O3"],
    deps = [
        ":coralnpu_architecture",
        ":coralnpu_simulator",
        ":coralnpu_v2_state",
        "@abseil-cpp//absl/flags:flag",
        "@abseil-cpp//absl/flags:marshalling",
        "@abseil-cpp//absl/flags:parse",
        "@abseil-cpp//absl/flags:usage",
        "@abseil-cpp//absl/log",
        "@abseil-cpp//absl/log:initialize",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/strings:str_format",
        "@abseil-cpp//absl/strings:string_view",
        "@abseil-cpp//absl/time",
        "@mpact-sim//mpact/sim/generic:type_helpers",
    ],
)

cc_library(
    name = "coralnpu_m4_instructions",
    srcs = ["coralnpu_m4_instructions.cc"],
    hdrs = ["coralnpu_m4_instructions.h"],
    copts = ["-O3"],
    deps = [
        ":coralnpu_v2_state",
        "@abseil-cpp//absl/log:check",
        "@mpact-riscv//riscv:riscv_state",
        "@mpact-riscv//riscv:riscv_zvt_instructions",
        "@mpact-riscv//riscv:riscv_zvt_state",
        "@mpact-sim//mpact/sim/generic:instruction",
        "@mpact-sim//mpact/sim/generic:type_helpers",
    ],
)

cc_library(
    name = "memory_config",
    hdrs = ["memory_config.h"],
    deps = [
        ":coralnpu_v2_state",
    ],
)

cc_library(
    name = "isg_engine",
    srcs = ["isg/isg_engine.cc"],
    hdrs = ["isg/isg_engine.h"],
    copts = ["-O3"],
    deps = [
        ":coralnpu_m3_isa",
        ":coralnpu_m3_user_decoder",
        ":coralnpu_v2_state",
        ":isg_watchdog",
        ":memory_config",
        "//sim/proto:isg_database_cc_proto",
        "@abseil-cpp//absl/base:no_destructor",
        "@abseil-cpp//absl/log:check",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/strings:str_format",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//:protobuf_lite",
        "@mpact-sim//mpact/sim/util/memory",
    ],
)

cc_library(
    name = "isg_watchdog",
    srcs = ["isg/isg_watchdog.cc"],
    hdrs = ["isg/isg_watchdog.h"],
    copts = ["-O3"],
    deps = [
        "@abseil-cpp//absl/status",
        "@abseil-cpp//absl/strings:str_format",
    ],
)

cc_test(
    name = "isg_engine_test",
    srcs = ["isg/isg_engine_test.cc"],
    copts = ["-O3"],
    data = [":coralnpu_m3_as"],
    deps = [
        ":coralnpu_simulator",
        ":isg_engine",
        "@abseil-cpp//absl/status:status_matchers",
        "@abseil-cpp//absl/strings",
        "@com_google_googletest//:gtest_main",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "coverage_summary",
    hdrs = ["coverage_summary.h"],
    copts = ["-O3"],
    deps = [
        "@abseil-cpp//absl/container:flat_hash_map",
    ],
)

cc_library(
    name = "fuzzer_types",
    hdrs = ["fuzzer_types.h"],
    copts = ["-O3"],
    deps = [
        ":coverage_summary",
        "@abseil-cpp//absl/container:flat_hash_map",
    ],
)

cc_library(
    name = "coverage_detector",
    hdrs = ["isg/coverage_detector.h"],
    copts = ["-O3"],
    deps = [
        "@mpact-sim//mpact/sim/generic:instruction",
    ],
)

cc_library(
    name = "coverage_event_router",
    hdrs = ["isg/coverage_event_router.h"],
    copts = ["-O3"],
    deps = [
        ":coverage_detector",
        "@mpact-sim//mpact/sim/generic:instruction",
    ],
)

cc_library(
    name = "execution_tracker",
    srcs = ["execution_tracker.cc"],
    hdrs = ["execution_tracker.h"],
    copts = ["-O3"],
    deps = [
        ":coralnpu_m3_isa",
        ":coverage_detector",
        "@abseil-cpp//absl/container:flat_hash_map",
        "@abseil-cpp//absl/container:flat_hash_set",
        "@abseil-cpp//absl/status:statusor",
        "@abseil-cpp//absl/strings",
        "@mpact-riscv//riscv:riscv_state",
        "@mpact-sim//mpact/sim/generic:instruction",
    ],
)

cc_library(
    name = "hazard_generator",
    srcs = ["isg/hazard_generator.cc"],
    hdrs = ["isg/hazard_generator.h"],
    copts = ["-O3"],
    deps = [
        ":coralnpu_m3_isa",
        ":coralnpu_m3_user_decoder",
        ":coralnpu_v2_state",
        ":isg_engine",
        ":memory_config",
        "@abseil-cpp//absl/memory",
        "@abseil-cpp//absl/strings",
        "@mpact-sim//mpact/sim/generic:instruction",
        "@mpact-sim//mpact/sim/util/memory",
    ],
)

cc_library(
    name = "behavioral_archive",
    srcs = ["behavioral_archive.cc"],
    hdrs = ["behavioral_archive.h"],
    copts = ["-O3"],
    deps = [
        ":coverage_summary",
        ":fuzzer_types",
        ":isg_engine",
        "@abseil-cpp//absl/container:flat_hash_map",
    ],
)

cc_test(
    name = "behavioral_archive_test",
    srcs = ["behavioral_archive_test.cc"],
    deps = [
        ":behavioral_archive",
        ":coverage_summary",
        "//sim/proto:isg_database_cc_proto",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "random_simulator",
    srcs = ["random_simulator.cc"],
    hdrs = ["random_simulator.h"],
    copts = ["-O3"],
    deps = [
        ":coralnpu_architecture",
        ":coralnpu_m3_assembler_lib",
        ":coralnpu_m3_isa",
        ":coralnpu_simulator",
        ":coverage_event_router",
        ":execution_tracker",
        ":fuzzer_types",
        ":isg_engine",
        ":memory_config",
        "//sim/proto:isg_database_cc_proto",
        "@abseil-cpp//absl/log",
        "@abseil-cpp//absl/status",
        "@abseil-cpp//absl/status:statusor",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/strings:str_format",
        "@com_github_serge1_elfio//:elfio",
        "@mpact-riscv//riscv:riscv_state",
        "@mpact-sim//mpact/sim/util/asm:simple_assembler",
    ],
)

cc_library(
    name = "orchestrator",
    srcs = ["orchestrator.cc"],
    hdrs = ["orchestrator.h"],
    copts = ["-O3"],
    deps = [
        ":behavioral_archive",
        ":fuzzer_types",
        ":isg_engine",
        ":random_simulator",
        "//sim/proto:isg_database_cc_proto",
        "@abseil-cpp//absl/log",
        "@abseil-cpp//absl/strings",
    ],
)

cc_binary(
    name = "coralnpu_random_sim",
    srcs = ["coralnpu_random_sim.cc"],
    copts = ["-O3"],
    deps = [
        ":orchestrator",
        ":random_simulator",
        "//sim/proto:isg_database_cc_proto",
        "@abseil-cpp//absl/flags:flag",
        "@abseil-cpp//absl/flags:parse",
        "@abseil-cpp//absl/log",
        "@abseil-cpp//absl/time",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "isg_database_parsing_test",
    srcs = ["isg/isg_database_parsing_test.cc"],
    data = [":coralnpu_random_sim"],
    deps = [
        "//sim/proto:isg_database_cc_proto",
        "@abseil-cpp//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "random_simulator_test",
    srcs = ["random_simulator_test.cc"],
    deps = [
        ":random_simulator",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "isg_database_export_test",
    srcs = ["isg/isg_database_export_test.cc"],
    data = [
        ":coralnpu_random_sim",
    ],
    deps = [
        "//sim/proto:isg_database_cc_proto",
        "@abseil-cpp//absl/log",
        "@abseil-cpp//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "isg_e2e_test",
    srcs = ["isg/isg_e2e_test.cc"],
    data = [
        ":coralnpu_m3_as",
    ],
    deps = [
        ":coralnpu_m3_isa",
        ":coralnpu_simulator",
        ":coverage_event_router",
        ":execution_tracker",
        ":hazard_generator",
        ":isg_engine",
        ":memory_config",
        "//sim/proto:isg_database_cc_proto",
        "@abseil-cpp//absl/log",
        "@abseil-cpp//absl/status",
        "@abseil-cpp//absl/status:status_matchers",
        "@abseil-cpp//absl/status:statusor",
        "@abseil-cpp//absl/strings",
        "@com_google_googletest//:gtest_main",
        "@mpact-riscv//riscv:riscv_state",
        "@mpact-riscv//riscv:riscv_top",
        "@mpact-sim//mpact/sim/generic:core",
        "@mpact-sim//mpact/sim/generic:instruction",
    ],
)

cc_test(
    name = "isg_integration_test",
    srcs = ["isg/isg_integration_test.cc"],
    data = [
        ":coralnpu_m3_as",
        ":coralnpu_m3_sim",
    ],
    tags = ["no_heapcheck"],
    deps = [
        ":coralnpu_m3_isa",
        ":coralnpu_simulator",
        ":coverage_event_router",
        ":execution_tracker",
        ":isg_engine",
        ":memory_config",
        "@abseil-cpp//absl/debugging:leak_check",
        "@abseil-cpp//absl/strings",
        "@com_google_googletest//:gtest_main",
        "@mpact-sim//mpact/sim/generic:core",
        "@mpact-sim//mpact/sim/generic:instruction",
    ],
)

cc_test(
    name = "isg_fp_poison_pill_test",
    srcs = ["isg/isg_fp_poison_pill_test.cc"],
    data = [
        ":coralnpu_m3_as",
    ],
    deps = [
        ":coralnpu_simulator",
        ":coverage_event_router",
        ":execution_tracker",
        ":isg_engine",
        "@abseil-cpp//absl/cleanup",
        "@abseil-cpp//absl/container:flat_hash_map",
        "@abseil-cpp//absl/status",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "orchestrator_test",
    srcs = ["orchestrator_test.cc"],
    data = [
        ":coralnpu_m3_as",
    ],
    deps = [
        ":isg_engine",
        ":memory_config",
        ":orchestrator",
        ":random_simulator",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "isg_watchdog_test",
    srcs = ["isg/isg_watchdog_test.cc"],
    deps = [
        ":isg_watchdog",
        "@abseil-cpp//absl/status",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "isg_database_test",
    srcs = ["isg/isg_database_test.cc"],
    deps = [
        ":coverage_summary",
        ":fuzzer_types",
        ":isg_database",
        "//sim/proto:isg_database_cc_proto",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "isg_toolchain_test",
    srcs = ["isg/isg_toolchain_test.cc"],
    data = ["@mpact-riscv//riscv:riscv64g_as"],
    deps = [
        ":isg_engine",
        "@abseil-cpp//absl/log",
        "@abseil-cpp//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "isg_database_integration_test",
    srcs = ["isg/isg_database_integration_test.cc"],
    deps = [
        ":isg_engine",
        "//sim/proto:isg_database_cc_proto",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "isg_mutator_test",
    srcs = ["isg/isg_mutator_test.cc"],
    deps = [
        ":isg_engine",
        ":isg_mutator",
        "//sim/proto:isg_database_cc_proto",
        "@abseil-cpp//absl/random",
        "@abseil-cpp//absl/strings:string_view",
        "@com_google_googletest//:gtest_main",
        "@mpact-sim//mpact/sim/generic:core",
    ],
)

cc_test(
    name = "coverage_event_router_test",
    srcs = ["isg/coverage_event_router_test.cc"],
    deps = [
        ":coverage_event_router",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "hazard_generator_test",
    srcs = ["isg/hazard_generator_test.cc"],
    deps = [
        ":hazard_generator",
        ":isg_engine",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/strings:str_format",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "isg_database",
    hdrs = ["isg/isg_database.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":coverage_summary",
        ":fuzzer_types",
        "//sim/proto:isg_database_cc_proto",
        "@abseil-cpp//absl/container:flat_hash_set",
    ],
)

cc_library(
    name = "isg_mutator",
    hdrs = ["isg/isg_mutator.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":coralnpu_m3_user_decoder",
        ":coralnpu_v2_state",
        ":isg_engine",
        ":memory_config",
        "//sim/proto:isg_database_cc_proto",
        "@abseil-cpp//absl/random",
        "@abseil-cpp//absl/status",
        "@mpact-sim//mpact/sim/generic:core",
        "@mpact-sim//mpact/sim/util/memory",
    ],
)

cc_library(
    name = "coralnpu_m3_instructions",
    srcs = [
        "coralnpu_m3_instructions.cc",
    ],
    hdrs = [
        "coralnpu_m3_instructions.h",
    ],
    copts = ["-O3"],
    deps = [
        ":coralnpu_state",
        "@abseil-cpp//absl/base",
        "@mpact-riscv//riscv:riscv_fp_state",
        "@mpact-riscv//riscv:riscv_g",
        "@mpact-riscv//riscv:riscv_state",
        "@mpact-sim//mpact/sim/generic:arch_state",
        "@mpact-sim//mpact/sim/generic:core",
        "@mpact-sim//mpact/sim/generic:instruction",
        "@mpact-sim//mpact/sim/generic:type_helpers",
    ],
)
