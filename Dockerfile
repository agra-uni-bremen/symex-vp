FROM alpine:3.18

RUN apk update && apk add --no-cache build-base cmake boost-dev z3-dev \
	llvm15-dev git gcc-riscv-none-elf newlib-riscv-none-elf

# Examples expect riscv32-unknown-* compiler triplet.
# TODO: Find a better way to deal with this problem.
RUN sh -c 'ln -s $(command -v riscv-none-elf-gcc) /usr/local/bin/riscv32-unknown-elf-gcc && \
           ln -s $(command -v riscv-none-elf-g++) /usr/local/bin/riscv32-unknown-elf-g++ && \
           ln -s $(command -v riscv-none-elf-as) /usr/local/bin/riscv32-unknown-elf-as && \
           ln -s $(command -v riscv-none-elf-ld) /usr/local/bin/riscv32-unknown-elf-ld'

RUN adduser -G users -g 'RISC-V VP User' -D riscv-vp
USER riscv-vp

ADD --chown=riscv-vp . /home/riscv-vp/riscv-vp
WORKDIR /home/riscv-vp/riscv-vp

RUN make -C /home/riscv-vp/riscv-vp
RUN sh -c "echo export PATH='\$PATH:/home/riscv-vp/riscv-vp/vp/build/bin' >> /home/riscv-vp/.profile"

# Run a login shell to force ash to source ~/.profile
ENTRYPOINT ["/bin/sh", "-l"]
