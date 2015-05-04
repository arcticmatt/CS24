.globl draw_pixel

# ===========================================================================
# draw_pixel: draws a pixel on a screen, if it is "closer" than the existing
#             pixel value
#
# Arguments to draw_pixel are at these stack locations:
#
#             8(%ebp) = Address of the screen array
#             12(%ebp) = x position of the pixel to draw
#             16(%ebp) = y position of the pixel to draw
#             20(%ebp) = value of the pixel to draw
#             24(%ebp) = depth of the pixel to draw
#
#
draw_pixel:
    pushl  %ebp              # Set up the stack
    movl   %esp, %ebp

    # Save callee-save registers we alter
    pushl  %ebx
    pushl  %esi
    pushl  %edi

    # Get width and height of screen
    movl   8(%ebp), %edx     # %edx = address of screen array, dereferencing
                             # should give us the width
    movl   (%edx), %ebx      # %ebx = M[%edx] = width
    addl   $4, %edx          # %edx += 4, dereferencing should give us the
                             # height
    movl   (%edx), %esi      # %esi = M[%edx] = height
    addl   $4, %edx          # %edx += 4, dereferencing should give us the
                             # first pixel

    # Calculate index of screen array to access to get the pixel we want
    movl   12(%ebp), %edi    # %edi = x
    movl   16(%ebp), %ecx    # %ecx = y

    # Make sure pixel coordinates are within bounds
    cmpl   $0, %edi          # Update flags as for x - 0
    jl     draw_pixel_return # If x < 0, return
    cmpl   $0, %ecx          # Update flags as for y - 0
    jl     draw_pixel_return # If y < 0, return
    cmpl   %edi, %ebx        # Update flags as for width - x
    jle    draw_pixel_return # If x >= width, return
    cmpl   %ecx, %esi        # Update flags as for height - y
    jle    draw_pixel_return # If y >= height, return

    # Calculate index of screen array to access to get the pixel we want
    imull  %ebx, %ecx        # %ecx = y * width
    addl   %edi, %ecx        # %ecx = y * width + x
    imull  $2, %ecx          # Multiply index by 2 because each pixel is 2 bytes

    addl   %ecx, %edx        # %edx += y * width + x, dereferencing should give
                             # us value of the pixel at coordinate (x, y)
    movl   %edx, %esi        # %esi = %edx; will now use %esi to access array
    movb   (%esi), %bl       # %bl = M[%esi] = old_pixel->value
    addl   $1, %esi          # %esi +=1, dereferencing should give us the depth
                             # of the pixel at coordinate (x, y)
    movb   (%esi), %dl       # %dl = M[%esi] = old_pixel->depth
    movb   20(%ebp), %cl     # %cl = new_pixel->value
    movb   24(%ebp), %al     # %al = new_pixel->depth
    movzbl %dl, %edi         # "Cast" al and dl to their unsigned values
    movzbl %al, %ebx
    cmpl   %ebx, %edi        # Updates flags as for %dl - %al, if they were
                             # cast to unsigned chars =>
                             # old_pixel->depth - new_pixel->depth
    jge    replace_pixel     # If old_pixel->depth >= new_pixel->depth, replace
                             # the pixel
    jmp    draw_pixel_return # Else, just return

replace_pixel:
    movb   %al, (%esi)       # Change the pixel depth to the passed-in value
    subl   $1, %esi          # Move %esi back so that it points to
                             # old_pixel->value
    movb   %cl, (%esi)       # Change the pixel value to the passed-in value

draw_pixel_return:
    xor    %eax, %eax        # Return 0

    # Restore the callee-save registers we altered
    popl   %edi
    popl   %esi
    popl   %ebx

    # Clean up stack, return
    movl   %ebp, %esp
    popl   %ebp
    ret
