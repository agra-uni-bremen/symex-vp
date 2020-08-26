.include "symex.h"

.globl _start
_start:

/* This calculation should always be performed concrete
 * That is: a3 should always be equal to 3 */
li t0, 1
li t1, 2
add a3, t0, t1

/* This calculation should always be performed on symbolic values */
MAKE_SYMBOLIC_REG 5 /* x5 → t0 */
blt t0, t1, true
addi a4, a4, 1
true:
addi a4, a4, 1

SYS_EXIT 0
