.globl get_ids

# ===========================================================================
# get_ids: implementation of get_ids(int *uid, int *gid), which retrieves
#          the user ID and stores it at the first address and then retrieves
#          the group ID and stores it at the second address.
#
#
# Arguments to get_ids are at these stack locations:
#
#         8(%ebp) = pointer to an integer to receive the user ID
#         12(%ebp) = pointer to an integer to receive the group ID
#
get_ids:
    push   %ebp
    mov    %esp, %ebp
    mov    8(%ebp), %ecx     # %ecx = &uid
    mov    12(%ebp), %edx    # %edx = &gid
    mov    $24, %eax
    int    $0x80             # Invoke system call 24 (getuid). Kernel stores
                             # process ID into eax.
    mov    %eax, (%ecx)      # M[%ecx] = %eax => M[&uid] = uid
    mov    $47, %eax
    int    $0x80             # Invoke system call 47 (getgid). Kernel stores
                             # process ID into eax.
    mov    %eax, (%edx)      # M[%edx] = %eax => M[&gid] = gid
    mov    %ebp, %esp
    pop    %ebp
    ret
