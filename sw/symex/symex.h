.macro SYS_EXIT, exit_code
li   a7, 93 /* SYS_exit */
li   a0, \exit_code
ecall
.endm

.macro SYS_ERROR
li   a7, 1 /* SYS_host_error */
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
