#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>


int main() {
    int val = 2;

    printf("%d", 0);
    fflush(stdout);

    if (fork() == 0) {
        val++;
        printf("%d", val);
        fflush(stdout);
    }
    else {
        val--;
        printf("%d", val);
        fflush(stdout);
        wait(NULL);
    }
    val++;
    printf("%d", val);
    fflush(stdout);
    exit(0);
}




/*pid_t pid;*/
/*int counter = 0;*/

/*void handler1(int sig) {*/
    /*counter++;*/
    /*printf("handler1: counter = %d\n", counter);*/
    /*fflush(stdout); [> Flushes the printed string to stdout <]*/
    /*kill(pid, SIGUSR1);*/
/*}*/

/*void handler2(int sig) {*/
    /*counter += 3;*/
    /*printf("handler2: counter = %d\n", counter);*/
    /*exit(0);*/
/*}*/

/*int main() {*/
    /*printf("pid = %d\n", pid);*/
    /*signal(SIGUSR1, handler1);*/
    /*if ((pid = fork()) == 0) {*/
        /*printf("CHILD\n");*/
        /*signal(SIGUSR1, handler2);*/
        /*kill(getppid(), SIGUSR1);*/
        /*while(1) {};*/
    /*} else {*/
        /*printf("PARENT\n");*/
        /*pid_t p;*/
        /*int status;*/
        /*if ((p = wait(&status)) > 0) {*/
            /*counter += 2;*/
            /*printf("counter = %d\n", counter);*/
        /*}*/
    /*}*/
/*}*/




/*int counter = 0;*/

/*int main() {*/
    /*int i;*/

    /*int child;*/
    /*for (i = 0; i < 2; i++) {*/
        /*child = fork();*/
        /*counter++;*/
        /*if (child == 0)*/
            /*printf("child counter = %d\n", counter);*/
        /*else*/
            /*printf("parent counter = %d\n", counter);*/
    /*}*/
    /*printf("counter = %d\n", counter);*/
    /*return 0;*/
/*}*/
