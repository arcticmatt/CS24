/*
 * Semaphores for the sthread package.
 *
 *--------------------------------------------------------------------
 * Adapted from code for CS24 by Jason Hickey.
 * Copyright (C) 2003-2010, Caltech.  All rights reserved.
 */
#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H

/*
 * The semaphore's contents are opaque to users of the semaphore.  The
 * actual definition of the semaphore type is within semaphore.c.
 */
typedef struct _semaphore Semaphore;

/*
 * ThreadWrapper handle.
 */
typedef struct _threadwrapper ThreadWrapper;

/*
 * Create a new seamphore.
 * The argument is the initial value of the semaphore.
 */
Semaphore *new_semaphore(int init);

/*
 * Decrement the semaphore.
 * This operation will block if the semaphore value is zero.
 */
void semaphore_wait(Semaphore *semp);

/*
 * Increment the semaphore.
 */
void semaphore_signal(Semaphore *semp);

/*
 * Add the passed-in thread to the blocked queue of the passed-in semaphore.
 */
void enqueue(Semaphore *semp, Thread *thread);

/*
 * Dequeue from the blocked queue of the passed-in semaphore.
 */
ThreadWrapper * dequeue(Semaphore *semp);

#endif /* _SEMAPHORE_H */

