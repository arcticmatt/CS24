.globl my_setjmp
.globl my_longjmp

# ===========================================================================
# my_setjmp: custom implementation of setjmp
#
#
# Arguments to my_setjmp are at these stack locations:
#
#         8(%ebp) = int array used to store execution state associated with
#                   registers and stack
#
# Return-value in %eax is always 0
#
my_setjmp:
