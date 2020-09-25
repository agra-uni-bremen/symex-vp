.include "symex.h"

.globl _start
_start:

MAKE_SYMBOLIC_REG 5 /* x5 → t0 */

beqz t0, true
SYS_EXIT 0 /* do nothing */
true:
SYS_ERROR

SYS_EXIT 0
