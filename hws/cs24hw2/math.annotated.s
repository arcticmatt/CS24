/* This file contains IA32 assembly-language implementations of three
 * basic, very common math operations.
 *
 *
 * The "common theme" is that all three of these implementations avoid using
 * conditional transfers of control.
 *
 * Naive implementations of f1 and would use a conditional transfer of control
 * instead of a conditional move instruction.
 *
 * A naive implementation of f2 would use a conditional transfer of control
 * instead of just using a fixed set of steps to get the desired result.
 *
 * The compiler wants to avoid using the naive implementations of these
 * functions because code based on conditional data transfers can outperform
 * code based on conditional control transfers. "This is because processors
 * achieve high performance through pipelining, which is where an instruction
 * is processed via a sequence of stages, each performing one small portion
 * of the required operations. This approach achieves high performance by
 * overlapping the steps of the successive instructions... to do this requires
 * being able to determine the sequence of instructions to be executed well
 * ahead of time in order to keep the pipeline full" (book). However,
 * conditional jumps require the processor to guess (branch prediction logic)
 * whether or not each jump instruction will be followed. Mispredicting a jump
 * causes the pipeline to be basically emptied, and time is wasted because the
 * processor cannot optimize the future instructions. However, with conditional
 * move instructions, we do not run into these problems, because we follow the
 * same set of instructions every time. The only difference is whether or not
 * we set one variable equal to another. This is why we want to avoid using
 * the naive implementations of f1 and f3. And in the case of f2, we avoid
 * conditionals completely; f2 has the same set of instructions executed every
 * time, avoiding the complication of branch prediction logic.
 */

    .text

/*====================================================================
 * int f1(int x, int y)
 * This is a "min" function. That is, it returns the minimum of x and y.
 */
.globl f1
f1:
	pushl	%ebp                # pushes ebp onto the stack
	movl	%esp, %ebp          # ebp = esp
	movl	8(%ebp), %edx       # edx = M[8 + ebp] = x
	movl	12(%ebp), %eax      # eax = M[12 + ebp] = y
	cmpl	%edx, %eax          # updates flags as for eax - edx = y - x
	cmovg	%edx, %eax          # sets eax = edx (sets eax = x) if y > x
	popl	%ebp                # pops the base pointer
	ret                         # pops top of stack into PC


/*====================================================================
 * int f2(int x)
 * This is an "abs value" function. That is, it returns the absolute value of x.
 */
.globl f2
f2:
	pushl	%ebp                # pushes ebp onto the stack
	movl	%esp, %ebp          # ebp = esp
	movl	8(%ebp), %eax       # eax = M[8 + ebp] = x
	movl	%eax, %edx          # edx = eax
	sarl	$31, %edx           # edx = edx >> 31 (arithmetic).
                                # if non-neg, get all 0s. if neg, get all 1s.
	xorl	%edx, %eax          # eax = eax ^ edx. if non-neg, get original
                                # input. if neg, get inverse input
	subl	%edx, %eax          # eax = eax - edx. if non-neg, eax = x - 0.
                                # if neg, eax = ~x - (-1) = ~x + 1 = -x
                                # (two's complement). so x = |x|
	popl	%ebp                # pops the base pointer
	ret                         # pops top of stack into PC


/*====================================================================
 * int f3(int x)
 * This is a "sign" function. That is, it returns the sign of x.
 * More specifically, if x > 0, it returns 1. If x = 0, it returns 0.
 * If x < 0, it returns -1.
 */
.globl f3
f3:
	pushl	%ebp                # pushes ebp onto the stack
	movl	%esp, %ebp          # ebp = esp
	movl	8(%ebp), %edx       # edx = M[8 + ebp] = x
	movl	%edx, %eax          # eax = edx
	sarl	$31, %eax           # eax = eax >> 31 (arithmetic). if non-neg,
                                # get all 0s. if neg, get all 1s.
	testl	%edx, %edx          # updates flags as for (edx & edx) = edx
	movl	$1, %edx            # edx = 1
	cmovg	%edx, %eax          # sets eax = edx (sets eax = 1) if
                                # (edx & edx) > 0. we have
                                # that (edx & edx) = edx. so we are really
                                # testing the sign of edx = x. if x > 0,
                                # eax = 1. if x = 0, then eax >> 31 was 0, and
                                # eax = 0. if x < 0, then eax >> 31 was all 1s,
                                # and eax = -1.
	popl	%ebp                # pops the base pointer
	ret                         # pops top of stack into PC

