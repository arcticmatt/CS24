#include <stdio.h>

int max(int x, int y);
int min(int x, int y);

int main(int argc, char **argv) {
    int arg1 = -1;
    int arg2 = -1;
    int pass1, pass2;
    int result;

    // Check for correct number of arguments
    if (argc > 1)
        arg1 = atoi(argv[1]);
    if (argc == 3) {
        arg2 = atoi(argv[2]);
        // Sort arguments we pass into gcd
        pass1 = max(arg1, arg2);
        pass2 = min(arg1, arg2);
    }

    // Only call gcd on positive arguments
    if (arg1 > 0 && arg2 > 0) {
        printf("Calling gcd(%d, %d)\n", pass1, pass2);
        result = gcd(pass1, pass2);
        printf("gcd(%d, %d) = %d\n", pass1, pass2, result);
    } else {
        printf("Invalid number of args or invalid args, arg1 = %d, \
                arg2 = %d, argc = %d\n", pass1, pass2, argc);
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
