.include "symex.h"

.globl _start
_start:

/* Make word (4 bytes) at addr 0x0 symbolic */
li t0, 0
li t1, 4
MAKE_SYMBOLIC_MEM t0, t1

/* Load symbolic memory word into register a1 */
lw   a1, 0(t0)

li a2, 23
blt  a2, a1, true
addi a3, a3, 1
true:
addi a3, a3, 2

/* Two branches should be found. a3 (x13) should either
 * have the value 2 or the the value 3. */
SYS_EXIT 0
