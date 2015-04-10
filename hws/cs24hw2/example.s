	.section	__TEXT,__text,regular,pure_instructions
	.globl	_ex
	.align	4, 0x90
_ex:                                    ## @ex
	.cfi_startproc
## BB#0:
	pushq	%rbp
Ltmp2:
	.cfi_def_cfa_offset 16
Ltmp3:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp4:
	.cfi_def_cfa_register %rbp
	movl	%edi, -4(%rbp)
	movl	%esi, -8(%rbp)
	movl	%edx, -12(%rbp)
	movl	%ecx, -16(%rbp)
	movl	-4(%rbp), %ecx
	movl	-8(%rbp), %edx
	subl	-12(%rbp), %edx # This is the - operation, where the two operands are b and c
	imull	%edx, %ecx      # This is the * operation, where the two operands are a and (b - c)
	addl	-16(%rbp), %ecx # This is the + operation, where the two operands are d and (a * (b - c))
	movl	%ecx, -20(%rbp)
	movl	-20(%rbp), %eax
	popq	%rbp
	retq
	.cfi_endproc


.subsections_via_symbols
