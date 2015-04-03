#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "ffunc.h"


/* This function takes an array of single-precision floating point values,
 * and computes a sum in the order of the inputs.  Very simple.
 */
float fsum(FloatArray *floats) {
    float sum = 0;
    int i;

    for (i = 0; i < floats->count; i++)
        sum += floats->values[i];

    return sum;
}


/* TODO:  IMPLEMENT my_fsum() HERE, AND DESCRIBE YOUR APPROACH. */
float my_fsum(FloatArray *floats) {
    float sum = 0;
    float next_sum;
    int i;
    float compensation;
    float sum1;
    float sum2;
    float comp1;
    float comp2;

    for (i = 0; i < floats->count; i++) {
        next_sum = sum + floats->values[i];
        // Need to parentheses because of float imprecision
        compensation = (next_sum - sum) - floats->values[i];
        /*printf("compensation = %f\n", compensation);*/
        if (fabs(compensation) < .00001) {
            sum = next_sum;
        } else {
            printf("compensation = %f\n", compensation);
            sum1 = sum + (floats->values[i] + compensation);
            sum2 = (sum + floats->values[i]) + compensation;
            comp1 = (sum1 - sum) - floats->values[i];
            comp2 = (sum2 - sum) - floats->values[i];
            printf("comp1 = %f\n", comp1);
            printf("comp2 = %f\n", comp2);
            if (fabs(comp1) < fabs(comp2))
                sum = sum1;
            else
                sum = sum2;
        }
    }

    return sum;
}


int main() {
    FloatArray floats;
    float sum1, sum2, sum3, my_sum;

    load_floats(stdin, &floats);
    printf("Loaded %d floats from stdin.\n", floats.count);

    /* Compute a sum, in the order of input. */
    sum1 = fsum(&floats);

    /* Use my_fsum() to compute a sum of the values.  Ideally, your
     * summation function won't be affected by the order of the input floats.
     */
    my_sum = my_fsum(&floats);

    /* Compute a sum, in order of increasing magnitude. */
    sort_incmag(&floats);
    sum2 = fsum(&floats);

    /* Compute a sum, in order of decreasing magnitude. */
    sort_decmag(&floats);
    sum3 = fsum(&floats);

    /* %e prints the floating-point value in full precision,
     * using scientific notation.
     */
    printf("Sum computed in order of input:  %e\n", sum1);
    printf("Sum computed in order of increasing magnitude:  %e\n", sum2);
    printf("Sum computed in order of decreasing magnitude:  %e\n", sum3);

    printf("My sum:  %e\n", my_sum);

    return 0;
}

