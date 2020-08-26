.include "symex.h"

.globl _start
_start:

addi a3, zero, 0 /* later used for result */

/* Make word (4 bytes) at addr 8 symbolic */
MAKE_SYMBOLIC_MEM 8, 4

addi a0, zero, 4
lw   a1, 4(a0)

blt  a0, a1, true
addi a3, a3, 1
true:
addi a3, a3, 2

/* Two branches should be found. a3 (x13) should either
 * have the value 2 or the the value 3. */
SYS_EXIT 0
