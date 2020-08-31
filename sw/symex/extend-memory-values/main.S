.include "symex.h"

.globl _start
_start:

sw zero, 0(zero)
lbu t0, 0(zero)
sw t0, 0(zero)

SYS_EXIT 0
