.globl fact
.text

gcd:
    pushl %ebp          # Push old base pointer
    movl %esp, %ebp     # Current stack is new base
    cmpl $0, 12(%ebp)   # Updates flags as for M[12 + ebp] - 0 = b - 0
    jne gcd_continue    # Here, b != 0, so we need to compute gcd(b, r)
    movl 8(%ebp), %eax  # Here, b == 0, and our answer is a, so we set eax = a
    jmp gcd_return      # And then we return

gcd_continue:
    movl 8(%ebp), %eax  # eax = a
    cltd                # Set up edx:eax for division by sign-extending eax into edx, creating edx:eax
    idivl 12(%ebp)      # eax = edx:eax / M[12 + ebp] = edx:eax / b = a / b
                        # edx = edx:eax mod M[12 + ebp] = a mod b
    pushl %edx          # Push remainder onto the stack
    pushl 12(%ebp)      # Push b onto the stack
    push %edx           # Dummy push so that addressing in gcd works out
    call gcd            # Make recursive call

gcd_return:
    movl %ebp, %esp     # Pop local stack
    popl %ebp           # Pop old base of frame
    ret
