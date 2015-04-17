.globl rl_decode

#============================================================================
# rl_decode:  decode RLE-encoded input into a malloc'd buffer
#
# Author:  Ben Bitdiddle (those guys are lucky I said I'd do this before
#          going on my vacation.  they never appreciate my work ethic!)
#
# Arguments to rl_decode are at these stack locations:
#
#      8(%ebp) = data buffer containing run-length encoded input data
#
#     12(%ebp) = length of the run-length-encoded data in the buffer
#
#     16(%ebp) = OUTPUT pointer to where the length of the decoded result
#                should be stored
#
# Return-value in %eax is the pointer to the malloc'd buffer containing
# the decoded data.
#
rl_decode:
        # Set up stack frame.
        push    %ebp
        mov     %esp, %ebp

        # Save callee-save registers.
        push    %ebx
        push    %esi
        push    %edi

        # First, figure out how much space is required to decode the data.
        # We do this by summing up the counts, which are in the odd memory
        # locations.

        mov     8(%ebp), %ecx             # %ecx = start of source array
        xor     %esi, %esi                # %esi = loop variable = 0
        xor     %ebx, %ebx                # %ebx = size required = 0

        # Find-space while-loop starts here...
        cmp     12(%ebp), %esi            # Updates flags as for esi - length = 0 - length
        jge     find_space_done           # if (-length >= 0) then jump to find_space_done

find_space_loop:
        add     (%ecx, %esi), %ebx        # Add in the count, then move.
                                          # bl is low 8 bits
                                          # bl = bl + M[ecx + esi]
                                          # b/c ecx is start of source array, esi is loop variable
        add     $2, %esi                  # forward to the next count!

        cmp     12(%ebp), %esi            # Updates flags as for esi - length = i - length,
                                          # where i is the loop variable
        jl      find_space_loop           # if (i - length < 0) then jump to find_space_loop

find_space_done:

        # Write the length of the decoded output to the output-variable
        mov     16(%ebp), %edx    # edx = last pointer-argument to function
        mov     %ebx, (%edx)      # store computed size into this location
                                  # M[edx = output pointer] = ebx

        # Allocate memory for the decoded data using malloc.
        # Pointer to allocated memory will be returned in %eax.
        push    %ebx              # Number of bytes to allocate...
        call    malloc
        add     $4, %esp          # Clean up stack after call.

        # Now, decode the data from the input buffer into the output buffer.
        xor     %esi, %esi        # esi = 0
        xor     %edi, %edi        # edi = 0

        # First comparison of decode while-loop here...
        cmp     12(%ebp), %esi    # Update flags as for esi - length = -length
        jge     decode_done       # if (-length >= 0) then jump to decode_done

decode_loop:
        # Pull out the next [count][value] pair from the encoded data.
        mov     (%ecx, %esi), %bh         # bh is the count of repetitions, high 8 bits of ebx
                                          # bh = M[ecx + esi]
        mov     1(%ecx, %esi), %bl        # bl is the value to repeat, low 8 bits of ebx
                                          # bl = [ecx + esi + 1]

write_loop:
        mov     %bl, (%eax, %edi)         # M[eax + edi] = bl. Remember that eax stores
                                          # the start of our output buffer
        dec     %bh                       # bh = bh - 1
        add     $1, edi                   # edi = edi + 1, because this is keeping place
                                          # of where we write to our output buffer
        cmp     $0, %bh                   # Updates flags as for bh - 0
        jnz     write_loop                #

        add     $2, %esi                  # esi = esi + 2

        cmp     12(%ebp), %esi            # Updates flags as for esi - length
        jl      decode_loop               # if (esi - length < 0) jump to decode_loop

decode_done:

        # Restore callee-save registers.
        pop     %edi
        pop     %esi
        pop     %ebx

        # Clean up stack frame.
        mov     %ebp, %esp
        pop     %ebp

        ret
