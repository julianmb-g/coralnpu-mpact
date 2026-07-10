FROM alpine:latest
RUN apk add --no-cache gcc-riscv-none-elf=15.2.0-r1 newlib-riscv-none-elf=4.5.0.20241231-r0 wget patch > /dev/null 2>&1
WORKDIR /opt/coremark
RUN wget -qO coremark.tar.gz https://github.com/eembc/coremark/archive/refs/tags/v1.01.tar.gz && \
    echo "99c5a6d63af85a281b4e4d6ccb522c446653c435dfec9455ad73ef9e71f28bde  coremark.tar.gz" | sha256sum -c - && \
    tar -xzf coremark.tar.gz --strip-components=1 && \
    rm coremark.tar.gz
