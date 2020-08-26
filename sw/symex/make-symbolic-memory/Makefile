CPPFLAGS += -I..

main: main.o
	riscv32-unknown-elf-ld -o $@ $<
main.o: main.S
	riscv32-unknown-elf-as $(CPPFLAGS) -o $@ $< -march=rv32i -mabi=ilp32
