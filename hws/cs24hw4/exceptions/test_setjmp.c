#include <stdio.h>
#include "my_setjmp.h"
#include "c_except.h"

static jmp_buf global_env;

void test1();
void test2();
void test3();
void test4();
void test5();
int f(int x);
int g(int x);
int h(int x);

/*
 * Tests initial value of setjmp(buf)
 */
void test1() {
    printf("========== Test 1 ==========\n");
    jmp_buf env;
    int exception = setjmp(env);
    if (exception == NO_EXCEPTION)
        printf("PASS\nsetjmp(buf) returns %d\n", exception);
    else
        printf("FAIL\nsetjmp(buf) returns %d\n", exception);
}

/*
 * Tests longjmp() return value when the ret argument is 0
 */
void test2() {
    printf("========== Test 2 ==========\n");
    jmp_buf env;
    int exception = setjmp(env);
    if (exception == NO_EXCEPTION) {
        longjmp(env, 0);
        printf("FAIL\nlongjmp did not jump back to setjmp\n");
    } else {
        if (exception == 1)
            printf("PASS\n");
        else
            printf("FAIL\n");
        printf("longjmp(buf, 0) returns %d\n", exception);
    }
}

/*
 * Tests longjmp() return value when the ret argument is n, n != 0
 */
void test3() {
    printf("========== Test 3 ==========\n");
    int n = 5;
    jmp_buf env;
    int exception = setjmp(env);
    if (exception == NO_EXCEPTION) {
        longjmp(env, n);
        printf("FAIL\nlongjmp did not jump back to setjmp\n");
    } else {
        if (exception == n)
            printf("PASS\n");
        else
            printf("FAIL\n");
        printf("longjmp(buf, %d) returns %d\n", n, exception);
    }
}

/*
 * Tests to see if setjmp() sticking within its proper range
 */
void test4() {
    printf("========== Test 4 ==========\n");
    int i = 3;
    int j = 4;
    jmp_buf env;
    int exception = setjmp(env);
    int k = 5;
    printf("i = %d, j = %d, k = %d\n", i, j, k);
    if (exception == NO_EXCEPTION) {
        k = 6;
        longjmp(env, 0);
        printf("FAIL\nlongjmp did not jump back to setjmp\n");
    } else {
        printf("Blah\n");
    }
}

/*
 * Tests that longjmp() can correctly jump across multiple function invocations
 */
void test5() {
    printf("========== Test 5 ==========\n");
    int result = f(5);
    if (result == -1)
        printf("PASS\n");
    else
        printf("FAIL\n");
    printf("Result = %d\n", result);
}

/*
 * Helper methods for test5
 */
int f(int x) {
    if (setjmp(global_env) == 0)
        return g(3 * x);
    else
        return -1;
}

int g(int x) {
    return h(15 - x);
}

int h(int x) {
    if (x < 5)
        longjmp(global_env, 1);

    return x - 5;
}


int main() {
    printf("Testing setjmp...\n");
    test1();
    test2();
    test3();
    /*test4();*/
    test5();

    return 0;
}
