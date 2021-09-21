.globl _start
.globl main
.globl symex_exit

_start:
jal main
j symex_exit
