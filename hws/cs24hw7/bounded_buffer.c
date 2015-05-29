/*
 * Define a bounded buffer containing records that describe the
 * results in a producer thread.
 *
 *--------------------------------------------------------------------
 * Adapted from code for CS24 by Jason Hickey.
 * Copyright (C) 2003-2010, Caltech.  All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>

#include "sthread.h"
#include "bounded_buffer.h"
#include "semaphore.h"

/*
 * The bounded buffer data.
 */
struct _bounded_buffer {
    /* The maximum number of elements in the buffer */
    int length;

    /* The index of the first element in the buffer */
    int first;

    /* The number of elements currently stored in the buffer */
    int count;

    /* The values in the buffer */
    BufferElem *buffer;

    /*
     * Semaphores, used for making bounded buffer thread safe.
     */
    Semaphore *semp_add;     // locks based on add
    Semaphore *semp_take;    // locks based on take
};

/*
 * For debugging, ensure that empty slots in the buffer are
 * set to a default value.
 */
static BufferElem empty = { -1, -1, -1 };

/*
 * Allocate a new bounded buffer.
 */
BoundedBuffer *new_bounded_buffer(int length) {
    BoundedBuffer *bufp;
    BufferElem *buffer;
    int i;

    /* Allocate the buffer */
    buffer = (BufferElem *) malloc(length * sizeof(BufferElem));
    bufp = (BoundedBuffer *) malloc(sizeof(BoundedBuffer));
    if (buffer == 0 || bufp == 0) {
        fprintf(stderr, "new_bounded_buffer: out of memory\n");
        exit(1);
    }

    /* Initialize */

    memset(bufp, 0, sizeof(BoundedBuffer));

    for (i = 0; i != length; i++)
        buffer[i] = empty;

    bufp->length = length;
    bufp->buffer = buffer;

    /*
     * NEW - allocate the sempahores
     */
    /* Start this one out with a length equivalent to how many items we can
     * store in our buffer. We do this because we will decrement this semp
     * everytime something gets added - thus, when the maximum number of
     * elements get added, we not be able to add anything until something
     * is taken
     */
    Semaphore *semp_add = new_semaphore(length);
    /*
     * Start this one out with a length of zero. We do this because initially,
     * the buffer has nothing in it. And since we decrement this semp in
     * the take method, we want it to block initially.
     */
    Semaphore *semp_take = new_semaphore(0);

    bufp->semp_add = semp_add;
    bufp->semp_take = semp_take;

    return bufp;
}

/*
 * Add an integer to the buffer.  Yield control to another
 * thread if the buffer is full.
 */
void bounded_buffer_add(BoundedBuffer *bufp, const BufferElem *elem) {
    /*
     * If there is space, we will add to the buffer. Else, this call will wait.
     */
    semaphore_wait(bufp->semp_add);

    /* Now the buffer has space */
    bufp->buffer[(bufp->first + bufp->count) % bufp->length] = *elem;
    bufp->count++;

    /*
     * Upon succesfully adding an element, increment the take semaphore because
     * we know we can take one more item from this buffer.
     */
    semaphore_signal(bufp->semp_take);
}

/*
 * Get an integer from the buffer.  Yield control to another
 * thread if the buffer is empty.
 */
void bounded_buffer_take(BoundedBuffer *bufp, BufferElem *elem) {
    /*
     * If there is an element to take, we will take it. Else, this call will
     * wait
     */
    semaphore_wait(bufp->semp_take);

    /* Copy the element from the buffer, and clear the record */
    *elem = bufp->buffer[bufp->first];
    bufp->buffer[bufp->first] = empty;
    bufp->count--;
    bufp->first = (bufp->first + 1) % bufp->length;

    /*
     * Upon succesfully taking an element, increment the add semaphore because
     * we know we can add one more item to this buffer.
     */
    semaphore_signal(bufp->semp_add);
}
