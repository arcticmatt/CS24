	.file	"gcd.c"
	.text
	.globl	gcd
	.type	gcd, @function
gcd:
.LFB0:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	cmpl	$0, 12(%ebp)
	jne	.L2
	movl	8(%ebp), %eax
	jmp	.L3
.L2:
	movl	8(%ebp), %eax
	cltd
	idivl	12(%ebp)
	movl	%edx, -12(%ebp)
	subl	$8, %esp
	pushl	-12(%ebp)
	pushl	12(%ebp)
	call	gcd
	addl	$16, %esp
.L3:
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE0:
	.size	gcd, .-gcd
	.ident	"GCC: (GNU) 4.9.2 20150304 (prerelease)"
	.section	.note.GNU-stack,"",@progbits
