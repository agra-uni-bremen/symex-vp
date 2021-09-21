CC := riscv32-unknown-elf-gcc
LD := riscv32-unknown-elf-ld

CFLAGS += -ggdb
CFLAGS += -march=rv32i -mabi=ilp32
CFLAGS += -nostartfiles

all: main
sim: main
	symex-vp $<

main: bootstrap.o main.o symex.o
	$(LD) -o $@ $^
bootstrap.o: bootstrap.S
	$(CC) -c $(CPPFLAGS) -o $@ $< $(CFLAGS)

%.o: %.c
	$(CC) -c $(CPPFLAGS) -o $@ $< $(CFLAGS) -nostartfiles

.PHONY: all sim
