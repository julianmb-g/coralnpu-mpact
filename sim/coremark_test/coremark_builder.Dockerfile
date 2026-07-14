FROM alpine:latest
RUN apk add --no-cache gcc-riscv-none-elf=15.2.0-r1 newlib-riscv-none-elf=4.5.0.20241231-r0 patch > /dev/null 2>&1
WORKDIR /opt/coremark
