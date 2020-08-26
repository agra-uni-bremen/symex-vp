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

.macro MAKE_SYMBOLIC_MEM, addr, size
li   a7, 96 /* SYS_sym_mem */
li   a0, \addr
li   a1, \size
ecall
.endm
