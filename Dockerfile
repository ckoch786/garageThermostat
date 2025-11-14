FROM debian:stable-slim
RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc-avr avr-libc gdb-avr binutils-avr \
    avrdude make cmake git ca-certificates && \
    rm -rf /var/lib/apt/lists/*
WORKDIR /work
