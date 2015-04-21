/*! \file
 * Implementation of a simple memory allocator.  The allocator manages a small
 * pool of memory, provides memory chunks on request, and reintegrates freed
 * memory back into the pool.
 *
 * Adapted from Andre DeHon's CS24 2004, 2006 material.
 * Copyright (C) California Institute of Technology, 2004-2010.
 * All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "myalloc.h"


/*!
 * These variables are used to specify the size and address of the memory pool
 * that the simple allocator works against.  The memory pool is allocated within
 * init_myalloc(), and then myalloc() and free() work against this pool of
 * memory that mem points to.
 */
int MEMORY_SIZE;
unsigned char *mem;

// Size of the header in bytes; used to store total size of memory chunk
static unsigned const int HEADER_SIZE = 4;
// Size of the footer in bytes; used to store total size of memory chunk
static unsigned const int FOOTER_SIZE = 4;
// Determined this constant by seeing what gives a relatively good memory
// utilization percentage. If we are allocating memory into a block and there is
// at least this much free space left, we will split the block.
static unsigned const int SPLIT_CRITERIA = 150;


/*!
 * This function initializes both the allocator state, and the memory pool.  It
 * must be called before myalloc() or myfree() will work at all.
 *
 * Note that we allocate the entire memory pool using malloc().  This is so we
 * can create different memory-pool sizes for testing.  Obviously, in a real
 * allocator, this memory pool would either be a fixed memory region, or the
 * allocator would request a memory region from the operating system (see the
 * C standard function sbrk(), for example).
 */
void init_myalloc() {

    /*
     * Allocate the entire memory pool, from which our simple allocator will
     * serve allocation requests.
     */
    mem = (unsigned char *) malloc(MEMORY_SIZE);
    if (mem == 0) {
        fprintf(stderr,
                "init_myalloc: could not get %d bytes from the system\n");
        abort();
    }

    /*
     * Initialize state of memory pool by writing an initial header and footer.
     * For all reasonably sized first allocations, this large block will be
     * split up upon the first allocation.
     */
    // Write header that contains the size of the entire memory chunk
    *((int *) mem) = -MEMORY_SIZE;
    // Write footer (to last 4 bytes) that contains the size of the entire
    // memory chunk
    *((int *) (mem + MEMORY_SIZE - FOOTER_SIZE)) = -MEMORY_SIZE;
}


/*!
 * Attempt to allocate a chunk of memory of "size" bytes.  Return 0 if
 * allocation fails.
 *
 * Time Complexity: O(k) where k is the number of blocks. This is because
 * we use a best fit algorithm, which checks all blocks to find the free block
 * that fits the size best. Thus we must iterate over all k blocks to find the
 * best free block. All other operations get put into the big O (splitting,
 * changing the header and footer, etc.).
 */
unsigned char *myalloc(int size) {
    printf("Seeking through memory for unused blocks\n");

    /*
     * We will use a BEST FIT placement strategy to traverse the sequence of
     * blocks.
     */
    int totalsize = size + HEADER_SIZE + FOOTER_SIZE;
    unsigned char *bestptr = 0;
    unsigned char *candidateptr = mem;
    int candidatesize = 0;
    int bestsize = MEMORY_SIZE;
    // Find best fit block
    while (candidateptr < mem + MEMORY_SIZE) {
        // Multiply by -1 because free blocks have -(size) in the header,
        // used blocks have just size in the header
        candidatesize = -1 * *((int *) candidateptr);
        if (candidatesize >= totalsize && candidatesize <= bestsize) {
            bestptr = candidateptr;
            bestsize = candidatesize;
        }
        candidateptr += abs(candidatesize);
    }

    if (bestptr != NULL) {
        /*
         * See if we want to split the block. Only split if there is a decent
         * amount of space left - this lessens fragmentation.
         */
        if (bestsize - totalsize > SPLIT_CRITERIA) {
            printf("Splitting the block\n");
            printf("First block has size %d\n", totalsize);
            // First Block. Make sure to write a positive size for header and
            // footer, because the first block is used.
            // Write header that contains the size of the entire memory chunk
            *((int *) bestptr) = totalsize;
            // Write footer (to last 4 bytes) that contains the size of the entire
            // memory chunk
            *((int *) (bestptr + size + HEADER_SIZE)) = totalsize;

            // Second block. Make sure to write a negative size for header and
            // footer, beause the second block is free..
            int second_block_size = bestsize - totalsize;
            printf("Second block has size %d\n", second_block_size);
            // Header
            *((int *) (bestptr + totalsize)) = -second_block_size;
            // Footer
            *((int *) (bestptr + totalsize + second_block_size - \
                        FOOTER_SIZE)) = -second_block_size;
        } else {
            /*
             * If we don't split the block, we just need to modify the sign of
             * the size, because we see no need to change the actual magnitude.
             * More specifically, we want to make the sign positive, because the
             * block is being used now.
             */
            // Write header that contains the size of the entire memory chunk
            *((int *) bestptr) = bestsize;
            // Write footer (to last 4 bytes) that contains the size of the entire
            // memory chunk
            *((int *) (bestptr + bestsize - FOOTER_SIZE)) = bestsize;
        }
        return (unsigned char *) (bestptr + HEADER_SIZE);
    } else {
        // Allocation failed
        fprintf(stderr, "myalloc: cannot service request of size %d\n", \
                size);
        return (unsigned char *) 0;
    }
}


/*
 * Free a previously allocated pointer. oldptr should be an address returned by
 * myalloc(). Coalesce adjacent blocks to get rid of false fragmentation.
 *
 * Here is the time complexity of each operation in this method:
 *
 * Deallocation: O(1). Deallocation just involves writing a number to the header
 * and footer of the block we are freeing. Since we know exactly where the
 * header and footer are in memory, these operations are constant time.
 *
 * Coalescing: O(1). Coalescing involves coalescing all blocks in front of the
 * block we are freeing and coalescing all blocks before the block we are
 * freeing. But this means that, at worst, we need to coalesce one block in
 * front and one block before. We will never run into the situation where we
 * have to coalesce multiple blocks in front or multiple blocks behind because
 * those blocks would already have been coalesced. So this means coalescing
 * is bounded above by coalescing one block in front and one block behind,
 * which just involves dereferencing a few pointers and adding some numbers.
 * So coalescing is constant time.
 */
void myfree(unsigned char *oldptr) {
    unsigned char *headerptr = (unsigned char *) (oldptr - HEADER_SIZE);
    int totalsize = *((int *) headerptr);

    printf("===== In myfree, with block of totalsize %d =====\n", totalsize);

    // Coalesce with free blocks after the just-freed block
    unsigned char *afterptr = (unsigned char *) (headerptr + totalsize);
    int currsize;
    while (afterptr < mem + MEMORY_SIZE) {
        currsize = *((int *) afterptr);
        if (currsize >= 0)
            break;
        printf("Coalescing forward with currsize = %d\n", currsize);
        totalsize += currsize * -1;
        afterptr += currsize * -1;
    }

    // Coalesce with free blocks before the just-freed block
    unsigned char *prev_footer_ptr = (unsigned char *) (headerptr - FOOTER_SIZE);
    int prev_total_size;
    while (prev_footer_ptr > mem) {
        prev_total_size = *((int *) prev_footer_ptr);
        if (prev_total_size >= 0)
            break;
        printf("Coalescing backward with prev_total_size = %d\n", \
              prev_total_size);
        totalsize += prev_total_size * -1;
        prev_footer_ptr -= prev_total_size * -1;
        headerptr -= prev_total_size * -1;
    }

    // Free the coalesced block by updating the header and footer. This is
    // the constant time deallocation.
    *((int *) headerptr) = totalsize * -1;
    *((int *) (headerptr + totalsize - 4)) = totalsize * -1;

    printf("Set the size to %d\n", *((int *) headerptr));
}
