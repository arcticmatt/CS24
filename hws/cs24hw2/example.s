	.file	"example.c"
	.text
	.globl	ex
	.type	ex, @function
ex:
.LFB0:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$16, %esp
	movl	12(%ebp), %eax
	subl	16(%ebp), %eax # This is the - operation, where the two operands are b and c
	imull	8(%ebp), %eax  # This is the * operation, where the two operands are a and (b - c)
	movl	%eax, %edx
	movl	20(%ebp), %eax
	addl	%edx, %eax     # This is the + operation, where the two operands are d and (a * (b - c))
	movl	%eax, -4(%ebp)
	movl	-4(%ebp), %eax
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE0:
	.size	ex, .-ex
	.ident	"GCC: (GNU) 4.9.2 20150304 (prerelease)"
	.section	.note.GNU-stack,"",@progbits
