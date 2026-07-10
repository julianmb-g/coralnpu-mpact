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
        "//sim/test:testfiles/core_portme.h",
        "//sim/test:testfiles/core_portme.c",
    ],
    outs = ["coremark_unified.S"],
    cmd = """
trap 'rm -f local_core_portme.h local_core_portme.c local_build_unified_asm.sh out_coremark_unified.S formatted_unified.S' EXIT
cp -L $(location //sim/test:testfiles/core_portme.h) local_core_portme.h
cp -L $(location //sim/test:testfiles/core_portme.c) local_core_portme.c
cp -L $(location build_unified_asm.sh) local_build_unified_asm.sh

podman run --userns=keep-id:uid=1000,gid=1000 --rm -v $$(pwd):/workspace -w /workspace localhost/coremark-builder:latest sh /workspace/local_build_unified_asm.sh
awk '{ sub(/^[[:space:]]+/, ""); if ($$0 == "" || (substr($$0, 1, 1) == "." && $$0 !~ /^\\.word[[:space:]]+0x/ && $$0 !~ /^\\.insn([[:space:]]|$$)/) || $$0 ~ /^[a-zA-Z0-9_.]+:/) print $$0; else print "  " $$0 }' out_coremark_unified.S > formatted_unified.S
mv formatted_unified.S $(location coremark_unified.S)
""",
    tags = ["local"],
    visibility = ["//sim/test:__pkg__"],
)

genrule(
    name = "coremark_individual_asm",
    srcs = [
        ":coremark_builder_image",
        "//sim/test:testfiles/core_portme.h",
        "//sim/test:testfiles/core_portme.c",
    ],
    outs = [
        "core_portme.S",
        "core_list_join.S",
        "core_state.S",
        "core_main.S",
        "core_util.S",
        "core_matrix.S",
    ],
    cmd = """
trap 'rm -f local_core_portme.h local_core_portme.c' EXIT
cp -L $(location //sim/test:testfiles/core_portme.h) local_core_portme.h
cp -L $(location //sim/test:testfiles/core_portme.c) local_core_portme.c

podman run --userns=keep-id:uid=1000,gid=1000 --rm -v $$(pwd):/workspace -w /workspace localhost/coremark-builder:latest sh -c '
wget -qO coremark.tar.gz https://github.com/eembc/coremark/archive/refs/tags/v1.01.tar.gz
echo "99c5a6d63af85a281b4e4d6ccb522c446653c435dfec9455ad73ef9e71f28bde  coremark.tar.gz" | sha256sum -c -
tar -xzf coremark.tar.gz
mv coremark-1.01 coremark_src

find coremark_src -name "core_portme.h" -delete
mv coremark_src/coremark.h coremark_src/coremark_authentic.h

grep -v "#define matrix_clip" coremark_src/core_matrix.c > coremark_src/core_matrix.c.tmp && mv coremark_src/core_matrix.c.tmp coremark_src/core_matrix.c
grep -v "#define matrix_big" coremark_src/core_matrix.c > coremark_src/core_matrix.c.tmp && mv coremark_src/core_matrix.c.tmp coremark_src/core_matrix.c
grep -v "#define bit_extract" coremark_src/core_matrix.c > coremark_src/core_matrix.c.tmp && mv coremark_src/core_matrix.c.tmp coremark_src/core_matrix.c

sed -i "s/0xe714/0x2ff5/g" coremark_src/core_main.c
sed -i "s/0x1fd7/0x6dfb/g" coremark_src/core_main.c
sed -i "s/0x8e3a/0x0000/g" coremark_src/core_main.c

mkdir coralnpu_port
cp /workspace/local_core_portme.h /workspace/local_core_portme.c coralnpu_port/
mv coralnpu_port/local_core_portme.h coralnpu_port/core_portme.h
mv coralnpu_port/local_core_portme.c coralnpu_port/core_portme.c
printf "#ifndef CORALNPU_COREMARK_H
#define CORALNPU_COREMARK_H

#include \\"coremark_authentic.h\\"

#if HAS_FLOAT
#undef MATDAT
#define MATDAT ee_f32
#undef MATRES
#define MATRES ee_f32
#endif

#endif
" > coralnpu_port/coremark.h

mkdir -p /tmp/build && cp -r coralnpu_port /tmp/build/ && cp -r coremark_src /tmp/build/
cd /tmp/build

riscv-none-elf-gcc -fno-exceptions -fno-builtin -fno-isolate-erroneous-paths-dereference -fno-isolate-erroneous-paths-attribute -fno-delete-null-pointer-checks -Wno-attributes -Wno-incompatible-pointer-types -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f -O3 -ftree-vectorize -fno-vect-cost-model -ffast-math -ffp-contract=off -DITERATIONS=500 -DHAS_FLOAT=1 -DEE_TYPES_DEFINED -I./coralnpu_port -I./coremark_src -S coralnpu_port/core_portme.c -o /workspace/core_portme.S &&
riscv-none-elf-gcc -fno-exceptions -fno-builtin -fno-isolate-erroneous-paths-dereference -fno-isolate-erroneous-paths-attribute -fno-delete-null-pointer-checks -Wno-attributes -Wno-incompatible-pointer-types -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f -O3 -ftree-vectorize -fno-vect-cost-model -ffast-math -ffp-contract=off -DITERATIONS=500 -DHAS_FLOAT=1 -DEE_TYPES_DEFINED -I./coralnpu_port -I./coremark_src -S coremark_src/core_list_join.c -o /workspace/core_list_join.S &&
riscv-none-elf-gcc -fno-exceptions -fno-builtin -fno-isolate-erroneous-paths-dereference -fno-isolate-erroneous-paths-attribute -fno-delete-null-pointer-checks -Wno-attributes -Wno-incompatible-pointer-types -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f -O3 -ftree-vectorize -fno-vect-cost-model -ffast-math -ffp-contract=off -DITERATIONS=500 -DHAS_FLOAT=1 -DEE_TYPES_DEFINED -I./coralnpu_port -I./coremark_src -S coremark_src/core_state.c -o /workspace/core_state.S &&
riscv-none-elf-gcc -fno-exceptions -fno-builtin -fno-isolate-erroneous-paths-dereference -fno-isolate-erroneous-paths-attribute -fno-delete-null-pointer-checks -Wno-attributes -Wno-incompatible-pointer-types -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f -O3 -ftree-vectorize -fno-vect-cost-model -ffast-math -ffp-contract=off -DITERATIONS=500 -DHAS_FLOAT=1 -DEE_TYPES_DEFINED -I./coralnpu_port -I./coremark_src -S coremark_src/core_main.c -o /workspace/core_main.S &&
riscv-none-elf-gcc -fno-exceptions -fno-builtin -fno-isolate-erroneous-paths-dereference -fno-isolate-erroneous-paths-attribute -fno-delete-null-pointer-checks -Wno-attributes -Wno-incompatible-pointer-types -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f -O3 -ftree-vectorize -fno-vect-cost-model -ffast-math -ffp-contract=off -DITERATIONS=500 -DHAS_FLOAT=1 -DEE_TYPES_DEFINED -I./coralnpu_port -I./coremark_src -S coremark_src/core_util.c -o /workspace/core_util.S &&
riscv-none-elf-gcc -fno-exceptions -fno-builtin -fno-isolate-erroneous-paths-dereference -fno-isolate-erroneous-paths-attribute -fno-delete-null-pointer-checks -Wno-attributes -Wno-incompatible-pointer-types -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f -O3 -ftree-vectorize -fno-vect-cost-model -ffast-math -ffp-contract=off -DITERATIONS=500 -DHAS_FLOAT=1 -DEE_TYPES_DEFINED -I./coralnpu_port -I./coremark_src -S coremark_src/core_matrix.c -o /workspace/core_matrix.S &&
cd /workspace
sed -E -i "s/\\.L([1-9][0-9]*)/\\.L_portme_\\1/g; s/\\.LC([0-9]+)/\\.LC_portme_\\1/g; s/\\.LANCHOR([0-9]+)/\\.LANCHOR_portme_\\1/g" core_portme.S &&
sed -E -i "s/\\.L([1-9][0-9]*)/\\.L_list_\\1/g; s/\\.LC([0-9]+)/\\.LC_list_\\1/g; s/\\.LANCHOR([0-9]+)/\\.LANCHOR_list_\\1/g" core_list_join.S &&
sed -E -i "s/\\.L([1-9][0-9]*)/\\.L_state_\\1/g; s/\\.LC([0-9]+)/\\.LC_state_\\1/g; s/\\.LANCHOR([0-9]+)/\\.LANCHOR_state_\\1/g" core_state.S &&
sed -E -i "s/\\.L([1-9][0-9]*)/\\.L_main_\\1/g; s/\\.LC([0-9]+)/\\.LC_main_\\1/g; s/\\.LANCHOR([0-9]+)/\\.LANCHOR_main_\\1/g" core_main.S &&
sed -E -i "s/\\.L([1-9][0-9]*)/\\.L_util_\\1/g; s/\\.LC([0-9]+)/\\.LC_util_\\1/g; s/\\.LANCHOR([0-9]+)/\\.LANCHOR_util_\\1/g" core_util.S &&
sed -E -i "s/\\.L([1-9][0-9]*)/\\.L_matrix_\\1/g; s/\\.LC([0-9]+)/\\.LC_matrix_\\1/g; s/\\.LANCHOR([0-9]+)/\\.LANCHOR_matrix_\\1/g" core_matrix.S &&
sed -i "/\\.attribute/d" core_portme.S core_list_join.S core_state.S core_main.S core_util.S core_matrix.S
'
mv core_portme.S $(location core_portme.S)
mv core_list_join.S $(location core_list_join.S)
mv core_state.S $(location core_state.S)
mv core_main.S $(location core_main.S)
mv core_util.S $(location core_util.S)
mv core_matrix.S $(location core_matrix.S)
    """,
    tags = ["local"],
    visibility = ["//sim/test:__pkg__"],
)

genrule(
    name = "coremark_individual_elf",
    srcs = [
        ":coremark_builder_image",
        ":coremark_individual_asm",
        "//sim/test:testfiles/crt0.S",
        "//sim/test:testfiles/linker.ld",
    ],
    outs = [
        "coremark_individual.elf",
        "coremark_individual.map",
        "coremark_individual.objdump",
    ],
    cmd = """
trap 'rm -f *.S *.o local_crt0.S local_linker.ld' EXIT
cp -L $(location //sim/test:testfiles/crt0.S) local_crt0.S &&
cp -L $(location //sim/test:testfiles/linker.ld) local_linker.ld &&
cp $(locations :coremark_individual_asm) .

podman run --userns=keep-id:uid=1000,gid=1000 --rm -v $$(pwd):/workspace -w /workspace localhost/coremark-builder:latest sh -c '
riscv-none-elf-gcc -c -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f core_portme.S -o core_portme.o &&
riscv-none-elf-gcc -c -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f core_list_join.S -o core_list_join.o &&
riscv-none-elf-gcc -c -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f core_state.S -o core_state.o &&
riscv-none-elf-gcc -c -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f core_main.S -o core_main.o &&
riscv-none-elf-gcc -c -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f core_util.S -o core_util.o &&
riscv-none-elf-gcc -c -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f core_matrix.S -o core_matrix.o &&
riscv-none-elf-gcc -fno-exceptions -fno-builtin -fno-isolate-erroneous-paths-dereference -fno-isolate-erroneous-paths-attribute -fno-delete-null-pointer-checks -u _printf_float -nostartfiles -Wno-attributes -Tlocal_linker.ld -Wl,-Map,coremark_individual.map -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f -O3 -ftree-vectorize -fno-vect-cost-model -ffast-math -ffp-contract=off -DITERATIONS=500 -DHAS_FLOAT=1 -DEE_TYPES_DEFINED local_crt0.S core_portme.o core_list_join.o core_state.o core_main.o core_util.o core_matrix.o -o coremark_individual.elf -lm &&
riscv-none-elf-objdump -d coremark_individual.elf > coremark_individual.objdump
'
mv coremark_individual.elf $(location coremark_individual.elf)
mv coremark_individual.map $(location coremark_individual.map)
mv coremark_individual.objdump $(location coremark_individual.objdump)
    """,
    tags = ["local"],
)

sh_test(
    name = "coralnpu_m3_coremark_individual_asm_test",
    srcs = ["run_validation.sh"],
    args = [
        "$(location :coremark_individual.elf)",
        "$(location //learning/brain/research/kelvin/sim:coralnpu_m3_sim)",
        "$(location :coremark_individual.objdump)",
    ],
    data = [
        "run_validation.sh",
        ":coremark_individual.elf",
        ":coremark_individual.objdump",
        "//sim:coralnpu_m3_sim",
    ],
    tags = ["local"],
)

genrule(
    name = "coremark_unified_elf",
    srcs = [
        ":coremark_builder_image",
        "build_unified_elf.sh",
        ":coremark_unified.S",
        "//sim/test:testfiles/linker.ld",
        "//sim/test:testfiles/crt0.S",
    ],
    outs = [
        "coremark_unified.elf",
        "coremark_unified.map",
        "coremark_unified.objdump",
    ],
    cmd = """
trap 'rm -f local_linker.ld local_crt0.S local_coremark_unified.S local_build_unified_elf.sh' EXIT
cp -L $(location //sim/test:testfiles/linker.ld) local_linker.ld &&
cp -L $(location //sim/test:testfiles/crt0.S) local_crt0.S &&
cp -L $(location :coremark_unified.S) local_coremark_unified.S &&
cp -L $(location build_unified_elf.sh) local_build_unified_elf.sh &&
podman run --userns=keep-id:uid=1000,gid=1000 --rm -v $$(pwd):/workspace -w /workspace localhost/coremark-builder:latest sh local_build_unified_elf.sh &&
mv coremark_unified_tmp.elf $(location coremark_unified.elf) &&
mv coremark_unified_tmp.map $(location coremark_unified.map) &&
mv coremark_unified_tmp.objdump $(location coremark_unified.objdump)
    """,
    tags = ["local"],
)

sh_test(
    name = "coralnpu_m3_coremark_unified_asm_test",
    srcs = ["run_validation.sh"],
    args = [
        "$(location :coremark_unified.elf)",
        "$(location //learning/brain/research/kelvin/sim:coralnpu_m3_sim)",
        "$(location :coremark_unified.objdump)",
    ],
    data = [
        "run_validation.sh",
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
        "diff_test.sh",
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
        "crt0_vector_test.sh",
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
        "$(location //learning/brain/research/kelvin/sim:coralnpu_m3_sim)",
        "$(location :coremark_builder_image)",
    ],
    data = [
        "run_validation_dynamic.sh",
        ":coremark_builder_image",
        "//sim:coralnpu_m3_sim",
        "//sim/test:testfiles/core_portme.c",
        "//sim/test:testfiles/core_portme.h",
        "//sim/test:testfiles/coremark_unified.S",
        "//sim/test:testfiles/crt0.S",
        "//sim/test:testfiles/linker.ld",
    ],
    tags = ["local"],
)

sh_test(
    name = "newlib_crt0_test",
    srcs = ["newlib_crt0_test.sh"],
    args = [
        "$(location //sim/test:testfiles/coremark_unified.S)",
        "$(location //sim/test:testfiles/linker.ld)",
        "$(location //learning/brain/research/kelvin/sim:coralnpu_m3_sim)",
        "$(location :coremark_builder_image)",
    ],
    data = [
        "newlib_crt0_test.sh",
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
        "$(location //learning/brain/research/kelvin/sim:coralnpu_m3_sim)",
        "$(location :coremark_builder_image)",
    ],
    data = [
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
        "$(location //learning/brain/research/kelvin/sim:coralnpu_m3_sim)",
        "$(location :coremark_builder_image)",
    ],
    data = [
        "portable_malloc_test.c",
        "portable_malloc_test.sh",
        ":coremark_builder_image",
        "//sim:coralnpu_m3_sim",
        "//sim/test:testfiles/core_portme.c",
        "//sim/test:testfiles/core_portme.h",
        "//sim/test:testfiles/crt0.S",
        "//sim/test:testfiles/linker.ld",
    ],
    tags = ["local"],
)
