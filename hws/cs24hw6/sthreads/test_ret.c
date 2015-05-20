#include <stdio.h>
#include "sthread.h"

/*
 * This thread-function runs a for-loop of the passed-in length, printing
 * the current index and yielding in each loop. After the for-loop is done,
 * the function returns.
 */
static void loop_for_length(void *length) {
    int len = *(int *) length;
    int i;
    for (i = 0; i < len; i++) {
        printf("loop index = %d\n", i);
        sthread_yield();
    }
    printf("loop_for_length done, returning\n");
    return;
}

/*
 * Tests running five threads, ensuring that each thread runs for a different
 * length of time, and also ensuring that each thread terminates by returning
 * from the thread-function. Should see the threads terminating one by one, and
 * when all the threads are completed the program should exit.
 */
int main(int argc, char **argv) {
    int a = 5;
    int b = 10;
    int c = 15;
    int d = 20;
    int e = 25;
    sthread_create(loop_for_length, &a);
    sthread_create(loop_for_length, &b);
    sthread_create(loop_for_length, &c);
    sthread_create(loop_for_length, &d);
    sthread_create(loop_for_length, &e);
    sthread_start();
    return 0;
}
