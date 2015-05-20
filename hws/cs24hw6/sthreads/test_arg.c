#include <stdio.h>
#include "sthread.h"

/*
 * This thread-function increments its argument and prints it over and over
 * again, yielding in each loop.
 */
static void increment(void *arg) {
    while(1) {
        int to_print = *(int *) arg;
        printf("arg_inc = %d\n", to_print);
        *((int *) arg) = to_print + 1;
        sthread_yield();
    }
}
/*
 * This thread-function decrements its argument and prints it over and over
 * again, yielding in each loop.
 */
static void decrement(void *arg) {
    while(1) {
        int to_print = *(int *) arg;
        printf("arg_dec = %d\n", to_print);
        *((int *) arg) = to_print - 1;
        sthread_yield();
    }
}

/*
 * Tests passing arguments to the thread-functions.
 */
int main(int argc, char **argv) {
    int a = 0;
    sthread_create(increment, &a);
    sthread_create(decrement, &a);
    sthread_start();
    return 0;
}
