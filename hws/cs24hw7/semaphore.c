/*
 * General implementation of semaphores.
 *
 *--------------------------------------------------------------------
 * Adapted from code for CS24 by Jason Hickey.
 * Copyright (C) 2003-2010, Caltech.  All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>

#include "sthread.h"
#include "semaphore.h"

/*
 * Holds a thread and pointers to next and previous ThreadWrappers. We do this
 * because when we add Threads to the queues in semaphores (SQueues) we do not
 * want to mutate the next and prev points of the Threads themselves, since
 * those pointers are used in the queue methods in sthread.c.
 */
struct _threadwrapper {
    Thread *thread;

    /*
     * The processes are linked in a doubly-linked list.
     */
    struct _threadwrapper *prev;
    struct _threadwrapper *next;
};

/*
 * The queue has a pointer to the head and the last element. The only difference
 * between this queue and the queue in sthread.c is that we store ThreadWrappers
 * here.
 */
typedef struct _squeue {
    ThreadWrapper *head;
    ThreadWrapper *tail;
} SQueue;

/*
 * The semaphore data structure contains a nonnegative integer variable which
 * gets either incremented or decremented. If the integer is nonzero, then
 * the decrement operation suceeds. Else, the decrement operation must wait
 * until the variable is nonzero.
 *
 * It also contains a queue of threads that the semaphore has blocked, in order
 * to implement fair unblocking.
 *
 * semaphore_wait decrements the integer (or waits)
 *
 * semaphore_signal increments the integer
 */
struct _semaphore {
    int count;
    SQueue *queue_b;
};

/************************************************************************
 * Top-level semaphore implementation.
 */

/*
 * Allocate a new semaphore.  The initial value of the semaphore is
 * specified by the argument.
 */
Semaphore *new_semaphore(int init) {
    Semaphore *semp;

    /*
     * Allocate and initialize a semaphore data struct with the passed-int count
     */

    // Make sure count is nonnegative
    assert(init >= 0);

    // Allocate and initialize semaphore
    semp = malloc(sizeof(Semaphore));
    semp->count = init;

    return semp;
}

/*
 * Decrement the semaphore.
 * This operation must be atomic, and it blocks iff the semaphore is zero.
 */
void semaphore_wait(Semaphore *semp) {
    // This operation must be atomic, so get a lock before attempting it
    int lock_result = __sthread_lock();
    while (lock_result == 0) {
        lock_result = __sthread_lock();
    }

    if (semp->count == 0) {
        // If the count is 0, block the thread. Don't increment the semp's
        // counter because we don't want to make it go negative
        sthread_block();
        // Add the blocked thread to the semaphore's queue
        enqueue(semp, sthread_current());
    } else {
        // Else, just decrement the semp's counter
        (semp->count)--;
    }

    // Make sure to unlock, because we got a lock at the beginning of this
    // method
    __sthread_unlock();
}

/*
 * Increment the semaphore.
 * This operation must be atomic.
 */
void semaphore_signal(Semaphore *semp) {
    // This operation must be atmoic, so get a lock before attempting it
    int lock_result = __sthread_lock();
    while (lock_result == 0) {
        lock_result = __sthread_lock();
    }

    // Just increment the semp's counter
    (semp->count)++;

    // Now, if a thread is blocked on this sempaphore, unblock one of them.
    // Do this fairly by retrieving the blocked thread to resume from the front
    // of the queue. Also, remove the thread from the blocked queue in
    // sthread.c
    ThreadWrapper *thread_wrapper = dequeue(semp);
    if (thread_wrapper != NULL && thread_wrapper->thread != NULL) {
        sthread_unblock(thread_wrapper->thread);
    }

    // Make sure to unlock, because we got a lock at the beginning of this
    // method
    __sthread_unlock();
}

/*
 * Add the passed-in thread to the blocked queue of the passed-in semaphore.
 */
void enqueue(Semaphore *semp, Thread *thread) {
   SQueue *queue = semp->queue_b;
   ThreadWrapper *thread_wrapper = malloc(sizeof(ThreadWrapper));
   thread_wrapper->thread = thread;

   if (queue->head == NULL) {
       // If semaphore's queue doesn't have a head, make the current
       // ThreadWrapper the head and tail
       thread_wrapper->prev = NULL;
       thread_wrapper->next = NULL;
       queue->head = thread_wrapper;
       queue->tail = thread_wrapper;
   } else {
       // Else, just add it to the end of the queue
       queue->tail->next = thread_wrapper;
       thread_wrapper->prev = queue->tail;
       thread_wrapper->next = NULL;
       queue->tail = thread_wrapper;
   }
}

/*
 * Dequeue from the blocked queue of the passed-in semaphore.
 */
ThreadWrapper * dequeue(Semaphore *semp) {
    SQueue *queue = semp->queue_b;
    ThreadWrapper *thread_wrapper;

    /* Return NULL if the queue is empty */
    if (queue->head == NULL)
        return NULL;

    /* Go to the final element */
    thread_wrapper = queue->head;
    if (thread_wrapper == queue->tail) {
        // Set head and tail to NULL
        queue->head = NULL;
        queue->tail = NULL;
    } else {
        // Change the head
        thread_wrapper->next->prev = NULL;
        queue->head = thread_wrapper->next;
    }
    return thread_wrapper;
}
