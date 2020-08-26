.include "symex.h"

.globl _start
_start:

addi x1, zero, 1
addi x1, x1, -1

/* Exit with exit code 0 */
SYS_EXIT 0
