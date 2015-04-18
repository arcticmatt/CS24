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


/* TODO:  The unacceptable allocator uses an external "free-pointer" to track
 *        where free memory starts.  If your allocator doesn't use this
 *        variable, get rid of it.
 *
 *        You can declare data types, constants, and statically declared
 *        variables for managing your memory pool in this section too.
 */
static unsigned char *freeptr;
static unsigned const int HEADER_SIZE = 4;
static unsigned const int FOOTER_SIZE = 4;
static unsigned const int SPLIT_CRITERIA = 100;


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

    /* TODO:  You can initialize the initial state of your memory pool here. */
    freeptr = mem;
}


/*!
 * Attempt to allocate a chunk of memory of "size" bytes.  Return 0 if
 * allocation fails.
 */
unsigned char *myalloc(int size) {
    int totalsize = size + HEADER_SIZE + FOOTER_SIZE;

    if (freeptr + totalsize < mem + MEMORY_SIZE) {
        printf("Allocating totalsize = %d with freeptr\n", totalsize);
        /*
         * Initially, we will use the freeptr to allocate memory
         */
        unsigned char *resultptr = freeptr;
        // Write header that contains the size of the entire memory chunk
        *((int *) resultptr) = totalsize;
        // Write footer (to last 4 bytes) that contains the size of the entire
        // memory chunk
        *((int *) (resultptr + size + HEADER_SIZE)) = totalsize;
        freeptr += totalsize;
        return (unsigned char *) (resultptr + HEADER_SIZE);
    } else {
        printf("Seeking through memory for unused blocks\n");
        /*
         * After freeptr runs to the edge of our memory pool, we will use
         * a BEST FIT placement strategy to traverse the sequence of blocks.
         */
        unsigned char *bestptr = 0;
        unsigned char *candidateptr = mem;
        int candidatesize = 0;
        int bestsize = MEMORY_SIZE;
        while (candidateptr < freeptr) {
            // Multiply by -1 because free blocks have -(size) in the header,
            // used blocks have just size in the header
            candidatesize = -1 * *((int *) candidateptr);
            /*printf("candidatesize = %d at candidateptr %p\n", candidatesize, candidateptr);*/
            if (candidatesize >= totalsize && candidatesize < bestsize) {
                bestptr = candidateptr;
                bestsize = candidatesize;
            }
            candidateptr += abs(candidatesize);
        }

        /*printf("here with bestpt = %p\n", bestptr);*/
        if (bestptr != NULL) {
            printf("Here with bestptr = %p of size %d for block of size %d\n", \
                    bestptr, bestsize, totalsize);
            // See if we want to split the block
            if (bestsize - totalsize > SPLIT_CRITERIA) {
                printf("Splitting the block\n");
                // First Block
                // Write header that contains the size of the entire memory chunk
                *((int *) bestptr) = totalsize;
                // Write footer (to last 4 bytes) that contains the size of the entire
                // memory chunk
                *((int *) (bestptr + size + HEADER_SIZE)) = totalsize;

                // Second block
                int second_block_size = bestsize - totalsize;
                // Header
                *((int *) (bestptr + totalsize)) = second_block_size;
                // Footer
                *((int *) (bestptr + totalsize + second_block_size - \
                            FOOTER_SIZE)) = second_block_size;
            }
            // If we don't split the block, we just need to modify the sign of
            // the size, because we see no need to change the actual magnitude
            // Write header that contains the size of the entire memory chunk
            *((int *) bestptr) = bestsize;
            // Write footer (to last 4 bytes) that contains the size of the entire
            // memory chunk
            *((int *) (bestptr + bestsize - FOOTER_SIZE)) = bestsize;
            return (unsigned char *) (bestptr + HEADER_SIZE);
        } else {
            fprintf(stderr, "myalloc: cannot service request of size %d with"
                    " %d bytes allocated\n", size, (freeptr - mem));
            return (unsigned char *) 0;
        }
    }
}


/*!
 * Free a previously allocated pointer.  oldptr should be an address returned by
 * myalloc().
 */
void myfree(unsigned char *oldptr) {

    unsigned char *headerptr = (unsigned char *) (oldptr - HEADER_SIZE);
    int totalsize = *((int *) headerptr);

    /*printf("in myfree, with block of totalsize %d\n", totalsize);*/

    // Coalesce with free blocks after the just-freed block
    unsigned char *afterptr = (unsigned char *) (headerptr + totalsize);
    int currsize;
    while (afterptr < freeptr) {
        currsize = *((int *) afterptr);
        if (currsize >= 0)
            break;
        /*printf("Coalescing forward with currsize = %d\n", currsize);*/
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
        /*printf("Coalescing backward with prev_total_size = %d\n", \*/
              /*prev_total_size);*/
        totalsize += prev_total_size * -1;
        prev_footer_ptr -= prev_total_size * -1;
        headerptr -= prev_total_size * -1;
    }

    *((int *) headerptr) = totalsize * -1;
    *((int *) (headerptr + totalsize - 4)) = totalsize * -1;

    /*printf("set the size to %d\n", *((int *) headerptr));*/
}
