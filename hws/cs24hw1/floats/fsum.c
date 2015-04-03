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


/*
 * Here we implement a more accurate float summing method
 * by introducing a compensation term into the summation.
 * Answering the questions in the set...
 * 1) Algebraically, the compensation term should always be equal to 0.
 * 2) It deviates from this expected value in situations where we are adding
 *    floats that differ a lot in magnitude.
 * 3) We can take advantage of the compensation term to improve accuracy
 *    by subtracting it from our next_sum term. This brings our next_sum term
 *    closer to what it actually should be.
 */
float my_fsum(FloatArray *floats) {
    float sum = 0;
    float next_sum;
    int i;
    float compensation = 0;

    for (i = 0; i < floats->count; i++) {
        // Apply compensation term
        next_sum = sum + (floats->values[i] - compensation);
        // Update compensation term; need parentheses because of float imprecision
        compensation += (next_sum - sum) - floats->values[i];
        sum = next_sum;
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

