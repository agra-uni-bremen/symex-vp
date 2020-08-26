.include "symex.h"

.globl _start
_start:

addi t0, zero, 0
addi a3, zero, 0

MAKE_SYMBOLIC_REG 6 /* x6 → t1 */

/* rs1 = t1
 * rs2 = t0
 *
 * rs1 >= rs2 */
bgeu t1, t0, true /* t1 >= 0 */

addi a3, a3, 1
true:
addi a3, a3, 5

/* Exit with exit status 0 */
SYS_EXIT 0
