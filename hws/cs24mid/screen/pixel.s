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
    push   %ebp              # Set up the stack
    mov    %esp, %ebp

    # Save callee-save registers we alter
    push   %ebx
    push   %esi
    push   %edi

    # Get width and height of screen
    mov    8(%ebp), %edx     # %edx = address of screen array, dereferencing
                             # should give us the width
    mov    (%edx), %ebx      # %ebx = M[%edx] = width
    add    $4, %edx          # %edx += 4, dereferencing should give us the
                             # height
    mov    (%edx), %esi      # %esi = M[%edx] = height
    add    $4, %edx          # %edx += 4, dereferencing should give us the
                             # array of pixels

    # Calculate index of screen array to access to get the pixel we want
    mov    12(%ebp), %edi    # %edi = x
    mov    16(%ebp), %ecx    # %ecx = y

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
    add    %edi, %ecx        # %ecx = y * width + x

    add    %ecx, %edx        # %edx += y * width + x, dereferencing should give
                             # us value of the pixel at coordinate (x, y)
    mov    (%edx), %ebx      # %ebx = M[%edx] = old_pixel->value
    add    $4, %edx          # %edx +=4, dereferencing should give us the depth
                             # of the pixel at coordinate (x, y)
    mov    (%edx), %esi      # %esi = M[%edx] = old_pixel->depth
    mov    20(%ebp), %ecx    # %ecx = new_pixel->value
    mov    24(%ebp), %edi    # %edi = new_pixel->depth
    cmpl   %edi, %esi        # Updates flags as for %esi - %edi =>
                             # old_pixel->depth - new_pixel->depth
    jge    replace_pixel     # If old_pixel->depth >= new_pixel->depth, replace
                             # the pixel
    jmp    draw_pixel_return # Else, just return

replace_pixel:
    sub    $4, %edx          # Move %edx back so that it points to
                             # old_pixel->value
    mov    %ecx, (%edx)      # Change the pixel value to the passed-in value
    add    $4, %edx          # Move %edx forward so that it points to
                             # old_pixel -> depth
    mov    %edi, (%edx)      # Change the pixel depth to the passed-in value

draw_pixel_return:
    xor    %eax, %eax        # Return 0

    # Restore the callee-save registers we altered
    pop    %edi
    pop    %esi
    pop    %ebx

    # Clean up stack, return
    movl   %ebp, %esp
    popl   %ebp
    ret
