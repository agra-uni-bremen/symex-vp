FROM alpine:edge

RUN apk update && apk add --no-cache -X "https://dl-cdn.alpinelinux.org/alpine/edge/testing" \
	build-base cmake boost-dev bash z3-dev llvm11-dev git gcc-riscv-none-elf newlib-riscv-none-elf
RUN adduser -G users -g 'RISC-V VP User' -D riscv-vp
ADD --chown=riscv-vp:users . /home/riscv-vp/riscv-vp
RUN su - riscv-vp -c 'make -C /home/riscv-vp/riscv-vp'
CMD su - riscv-vp
