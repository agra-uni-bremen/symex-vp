.macro GET_DRAM_START, register
/* 2 << 30 == 0x80000000 (DRAM start address) */
li   \register, 2
slli \register, \register, 30
.endm

.macro SYS_EXIT, exit_code
li   a7, 93 /* SYS_exit */
li   a0, \exit_code
ecall
.endm

.macro MAKE_SYMBOLIC_REG, register
li   a7, 95 /* SYS_sym_reg */
li   a0, \register
ecall
.endm

.macro MAKE_SYMBOLIC_MEM, addr_register, size_register
li   a7, 96 /* SYS_sym_mem */
mv   a0, \addr_register
mv   a1, \size_register
ecall
.endm
