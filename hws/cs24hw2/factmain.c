#include <stdio.h>

int main(int argc, char **argv) {
    int arg = -1;
    int result;
    if (argc > 1)
        arg = atoi(argv[1]);

    if (arg >= 0) {
        printf("Calling fact(%d)\n", arg);
        result = fact(arg);
        printf("fact(%d) = %d\n", arg, result);
    } else {
        printf("Invalid number of args or invalid arg, arg = %d, \
                argc = %d\n", arg, argc);
    }
}
