#include <stdio.h>

int max(int x, int y);
int min(int x, int y);

int main(int argc, char **argv) {
    int arg1 = -1;
    int arg2 = -1;

    if (argc == 3) {
        arg1 = atoi(argv[1]);
        arg2 = atoi(argv[2]);
        arg1 = max(arg1, arg2);
        arg2 = min(arg1, arg2);
    }

    if (arg1 >= 0 && arg2 >= 0) {
        printf("Calling gcd(%d, %d)\n", arg1, arg2);
    } else {
        printf("Invalid number of args or invalid args, arg1 = %d, \
                arg2 = %d, argc = %d\n", arg1, arg2, argc);
    }
}

int max(int x, int y) {
    int ans = x > y ? x : y;
    return ans;
}

int min(int x, int y) {
    int ans = x < y ? x : y;
    return ans;
}
