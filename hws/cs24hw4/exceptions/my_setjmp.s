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
    mov   %esp, 8(%ebp)                # M[8 + %ebp + 0] = %esp =>
                                       # first element of int array is the
                                       # current stack pointer (do this before
                                       # we push stuff onto the stack)
    push  %eax                         # Use as temporary variable
    push  %ebx                         # Use as array indexer
    xor   %ebx, %ebx                   # %ebx = 0
    add   $4, %ebx                     # %ebx = 4 b/c we already added the
                                       # stack pointer to our array
    mov   (%ebp), %eax
    mov   %eax, 8(%ebp, %ebx)          # M[8 + %ebp + 4] = M[%ebp] =>
                                       # second element of int array is the
                                       # caller's %ebp
    add   $4, %ebx                     # Add 4 because we have an int array
    mov   4(%ebp), %eax
    mov   %eax, 8(%ebp, %ebx)          # M[8 + %ebp + 8] = M[4 + %ebp] =>
                                       # third element of int array is the
                                       # caller's return address
    add   $4, %ebx
    mov   %esi, 8(%ebp, %ebx)          # M[8 + %ebp + 12] = %esi =>
                                       # fourth element of int array is the
                                       # callee-save register %esi
    add   $4, %ebx
    mov   %edi, 8(%ebp, %ebx)          # M[8 + %ebp + 16] = %edi =>
                                       # fifth element of int array is the
                                       # callee-save register %edi
    add   $4, %ebx
    mov   (%esp), %eax                 # Move M[%esp] = %ebx into %eax
    mov   %eax, 8(%ebp, %ebx)          # M[8 + %ebp + 20] = %ebx =>
                                       # sixth element of int array is the
                                       # callee-save register %ebx
    pop   %ebx                         # Restore %ebx
    pop   %eax                         # Restore %eax
    call  curr

curr:
    pop   %eax                         # Gets address of curr
    push  %eax                         # Push address onto stack
    xor   %eax, %eax                   # %eax = 0
    ret                                # Return to point in code after setjmp

my_longjmp:

