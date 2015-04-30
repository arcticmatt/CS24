#include <stdio.h>
#include "c_except.h"

static jmp_buf global_env;

void test1();
void test2();
void test3();
void test4();
void test5();
void test6();
void test7();
int f(int x);
int g(int x);
int h(int x);

/*
 * Tests initial value of setjmp(buf). That is, tests whether the initial
 * call to setjmp(buf) returns 0.
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
 * Tests longjmp() return value when the ret argument is 0. Also tests whether
 * or not longjmp() actually longjmp() actually jumps back in the code; if it
 * does not perform this behavior, and the line after longjmp() executes, this
 * tests fails.
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
            printf("PASS\nlongjmp(buf, 0) returns %d\n", exception);
        else
            printf("FAIL\nlongjmp(buf, 0) returns %d\n", exception);
    }
}

/*
 * Tests longjmp() return value when the ret argument is n, n != 0. Also tests
 * whether or not longjmp() actually longjmp() actually jumps back in the code;
 * if it does not perform this behavior, and the line after longjmp() executes,
 * this tests fails.
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
 * Tests nested/multiple setjmp()/longjmp() calls, to make sure they behave
 * as expected when put together.
 */
void test4() {
    printf("========== Test 4 ==========\n");
    int m = 4;
    int n = 5;
    jmp_buf env1;
    jmp_buf env2;
    int exception1, exception2;
    exception1 = setjmp(env1);
    if (exception1 == NO_EXCEPTION) {
        exception2 = setjmp(env2);
        if (exception2 == NO_EXCEPTION) {
            longjmp(env1, m); // 1) Jumps here
            printf("FAIL\nlongjmp(env1, m) did not jump back to setjmp\n");
        } else if (exception2 == n) {
            longjmp(env1, n); // 3) Jumps here
            printf("FAIL\nlongjmp(env1, n) did not jump back to setjmp\n");
        } else {
            printf("FAIL\nsetjmp(env2) returns %d\n", exception2);
        }
    } else if (exception1 == m) {
        longjmp(env2, n);     // 2) Jumps here
        printf("FAIL\nlongjmp(env2, n) did not jump back to setjmp\n");
    } else if (exception1 == n) {
        printf("PASS\nexception1 is %d\n", exception1); // 4) Jumps here
    } else {
        printf("FAIL\nexception1 is %d\n", exception1);
    }
}

/*
 * Tests for stack corruption by putting a local variable with a known value
 * before the jmp_buf variable to ensure that setjmp() doesn't go beyond the
 * extent of the mp_buf.
 */
void test5() {
    printf("========== Test 5 ==========\n");
    int x = 5;
    jmp_buf env;
    int exception = setjmp(env);
    if (exception == NO_EXCEPTION) {
        longjmp(env, 0);
        printf("FAIL\nlongjmp did not jump back to setjmp\n");
    } else {
        if (x == 5)
            printf("PASS\nx maintained value of %d\n", x);
        else
            printf("FAIL\nvalue of x changed to %d\n", x);
    }
}

/*
 * Tests for stack corruption by putting a local variable with a known value
 * after the jmp_buf variable to ensure that setjmp() doesn't go beyond the
 * extent of the mp_buf. Also tests to make sure that if we change that variable
 * before longjmp()-ing, that value is retained.
 */
void test6() {
    printf("========== Test 6 ==========\n");
    jmp_buf env;
    int x = 5;
    int exception = setjmp(env);
    if (exception == NO_EXCEPTION) {
        x = 6;
        longjmp(env, 0);
        printf("FAIL\nlongjmp did not jump back to setjmp\n");
    } else {
        if (x == 6)
            printf("PASS\nx maintained value of %d\n", x);
        else
            printf("FAIL\nvalue of x changed to %d\n", x);
    }
}

/*
 * Tests that longjmp() can correctly jump across multiple function invocations.
 */
void test7() {
    printf("========== Test 7 ==========\n");
    int result = f(5);
    if (result == -1)
        printf("PASS\n");
    else
        printf("FAIL\n");
    printf("Result = %d\n", result);
}

/*
 * Helper methods for test6
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

    // Run all the tests
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();

    return 0;
}
