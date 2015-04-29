#include "alloc.h"
#include "ptr_vector.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


/*! Change to #define to output garbage-collector statistics. */
#define GC_STATS

/*!
 * Change to #undef to cause the garbage collector to only run when it has to.
 * This dramatically improves performance.
 *
 * However, while testing GC, it's easiest if you try it all the time, so that
 * the number of objects being manipulated is small and easy to understand.
 */
#define ALWAYS_GC


/* Change to #define for other verbose output. */
#undef VERBOSE


void free_value(Value *v);
void free_lambda(Lambda *f);
void free_environment(Environment *env);

/* Marking functions */
void mark_environment(Environment *env);
void mark_value(Value *v);
void mark_lambda(Lambda *f);
void mark_eval_stack(PtrStack *eval_stack);

/* Sweeping functions */
void sweep_values();
void sweep_lambdas();
void sweep_environments();


/*========================================================*
 * TODO:  Declarations of your functions might go here... *
 *========================================================*/


/*!
 * A growable vector of pointers to all Value structs that are currently
 * allocated.
 */
static PtrVector allocated_values;


/*!
 * A growable vector of pointers to all Lambda structs that are currently
 * allocated.  Note that each Lambda struct will only have ONE Value struct that
 * points to it.
 */
static PtrVector allocated_lambdas;


/*!
 * A growable vector of pointers to all Environment structs that are currently
 * allocated.
 */
static PtrVector allocated_environments;


#ifndef ALWAYS_GC

/*! Starts at 1MB, and is doubled every time we can't stay within it. */
static long max_allocation_size = 1048576;

#endif


void init_alloc() {
    pv_init(&allocated_values);
    pv_init(&allocated_lambdas);
    pv_init(&allocated_environments);
}


/*!
 * This helper function prints some helpful details about the current allocation
 * status of the program.
 */
void print_alloc_stats(FILE *f) {
    /*
    fprintf(f, "Allocation statistics:\n");
    fprintf(f, "\tAllocated environments:  %u\n", allocated_environments.size);
    fprintf(f, "\tAllocated lambdas:  %u\n", allocated_lambdas.size);
    fprintf(f, "\tAllocated values:  %u\n", allocated_values.size);
    */

    fprintf(f, "%d vals \t%d lambdas \t%d envs\n", allocated_values.size,
        allocated_lambdas.size, allocated_environments.size);
}


/*!
 * This helper function returns the amount of memory currently being used by
 * garbage-collected objects.  It is NOT the total amount of memory being used
 * by the interpreter!
 */
long allocation_size() {
    long size = 0;

    size += sizeof(Value) * allocated_values.size;
    size += sizeof(Lambda) * allocated_lambdas.size;
    size += sizeof(Value) * allocated_environments.size;

    return size;
}


/*!
 * This function heap-allocates a new Value struct, initializes it to be empty,
 * and then records the struct's pointer in the allocated_values vector.
 */
Value * alloc_value(void) {
    Value *v = malloc(sizeof(Value));
    memset(v, 0, sizeof(Value));

    pv_add_elem(&allocated_values, v);

    return v;
}


/*!
 * This function frees a heap-allocated Value struct.  Since a Value struct can
 * represent several different kinds of values, the function looks at the
 * value's type tag to determine if additional memory needs to be freed for the
 * value.
 *
 * Note:  It is assumed that the value's pointer has already been removed from
 *        the allocated_values vector!  If this is not the case, serious errors
 *        will almost certainly occur.
 */
void free_value(Value *v) {
    assert(v != NULL);

    /*
     * If value refers to a lambda, we don't free it here!  Lambdas are freed
     * by the free_lambda() function, and that is called when cleaning up
     * unreachable objects.
     */

    if (v->type == T_String || v->type == T_Atom || v->type == T_Error)
        free(v->string_val);

    free(v);
}



/*!
 * This function heap-allocates a new Lambda struct, initializes it to be empty,
 * and then records the struct's pointer in the allocated_lambdas vector.
 */
Lambda * alloc_lambda(void) {
    Lambda *f = malloc(sizeof(Lambda));
    memset(f, 0, sizeof(Lambda));

    pv_add_elem(&allocated_lambdas, f);

    return f;
}


/*!
 * This function frees a heap-allocated Lambda struct.
 *
 * Note:  It is assumed that the lambda's pointer has already been removed from
 *        the allocated_labmdas vector!  If this is not the case, serious errors
 *        will almost certainly occur.
 */
void free_lambda(Lambda *f) {
    assert(f != NULL);

    /* Lambdas typically reference lists of Value objects for the argument-spec
     * and the body, but we don't need to free these here because they are
     * managed separately.
     */

    free(f);
}


/*!
 * This function heap-allocates a new Environment struct, initializes it to be
 * empty, and then records the struct's pointer in the allocated_environments
 * vector.
 */
Environment * alloc_environment(void) {
    Environment *env = malloc(sizeof(Environment));
    memset(env, 0, sizeof(Environment));

    pv_add_elem(&allocated_environments, env);

    return env;
}


/*!
 * This function frees a heap-allocated Environment struct.  The environment's
 * bindings are also freed since they are owned by the environment, but the
 * binding-values are not freed since they are externally managed.
 *
 * Note:  It is assumed that the environment's pointer has already been removed
 *        from the allocated_environments vector!  If this is not the case,
 *        serious errors will almost certainly occur.
 */
void free_environment(Environment *env) {
    int i;

    /* Free the bindings in the environment first. */
    for (i = 0; i < env->num_bindings; i++) {
        free(env->bindings[i].name);
        /* Don't free the value, since those are handled separately. */
    }
    free(env->bindings);

    /* Now free the environment object itself. */
    free(env);
}

/*!
 * This function marks the passed-in environment, its parent environment (if
 * non-null), and the bindings of the passed-in environment.
 */
void mark_environment(Environment *env) {
    // If already marked, return
    if (env->marked == 1)
        return;

    int i;

    // Mark parent env if it is non-null and not marked
    if (env->parent_env != NULL)
        mark_environment(env->parent_env);

    // Now go through variable bindings of env and mark all those
    for (i = 0; i < env->num_bindings; i++)
        mark_value(env->bindings[i].value);

    // Actually mark the passed-in env
    env->marked = 1;
}

/*!
 * This function marks the passed-in value, making extra calls if the value
 * is of the type T_Lambda or T_ConsPair (a call to mark_lambda or recursive
 * calls, respectively).
 */
void mark_value(Value *v) {
    // If already marked, return
    if (v->marked == 1)
        return;

    if (v->type == T_Lambda) {
        // Mark the lambda function
        mark_lambda(v->lambda_val);
    } else if (v->type == T_ConsPair) {
        // Recursively call mark_value for the values stored in the cons pair
        mark_value(v->cons_val.p_car);
        mark_value(v->cons_val.p_cdr);
    }
    // All other types are either char *, int, or float, so we can just
    // mark the passed-in value and be done
    v->marked = 1;
}

/*!
 * This function marks the passed-in lambda, the lambda's parent environment,
 * and (if the lambda is an interpreted lambda) the list of args and the body
 * values.
 */
void mark_lambda(Lambda *f) {
    // If already marked, return
    if (f->marked == 1)
        return;

    // Lambdas have pointers to their parent env. Mark it.
    mark_environment(f->parent_env);

    // If the lambda is an interpreted lambda (not native), we need to mark
    // the list of args (arg_spec) and the body (body) values
    if (f->native_impl == 0) {
        mark_value(f->arg_spec);
        mark_value(f->body);
    }

    // Mark all lambdas, whether they are native or interpreted
    f->marked = 1;
}

/*!
 * This function goes through the evaluation stack and marks all the evaluation
 * contexts that the stack stores. Marking an evaluation context involves
 * marking the current_env, expression, and child_eval_result associated
 * with the evaluation context. It also involves marking the local_vals
 * PtrVector associated with the evaluation context; to do this, we need to
 * loop through the PtrVector and mark each value.
 */
void mark_eval_stack(PtrStack *eval_stack) {
    int i;
    int j;

    // Loop through elements in eval_stack
    for (i = 0; i < eval_stack->size; i++) {
       EvaluationContext *ctx = pv_get_elem(eval_stack, i);

       // Mark the environment, if not null and if not already marked
       Environment *env = ctx->current_env;
       if (env != NULL)
           mark_environment(env);

       // Mark the expression, if not null
        Value *expr = ctx->expression;
        if (expr != NULL)
            mark_value(expr);

        // Mark the child_eval_result, if not null
        Value *child_eval_result = ctx->child_eval_result;
        if (child_eval_result != NULL)
            mark_value(child_eval_result);

        // Loop through local vals and mark all the values
        PtrVector *local_vals = &ctx->local_vals;
        for (j = 0; j < local_vals->size; j++) {
            Value **ppv = (Value **) pv_get_elem(local_vals, j);
            if (*ppv != NULL)
                mark_value(*ppv);
        }
    }
}


/*!
 * This function performs the garbage collection for the Scheme interpreter.
 * It also contains code to track how many objects were collected on each run,
 * and also it can optionally be set to do GC when the total memory used grows
 * beyond a certain limit.
 */
void collect_garbage() {
    Environment *global_env;
    PtrStack *eval_stack;

#ifdef GC_STATS
    int vals_before, procs_before, envs_before;
    int vals_after, procs_after, envs_after;

    vals_before = allocated_values.size;
    procs_before = allocated_lambdas.size;
    envs_before = allocated_environments.size;
#endif

#ifndef ALWAYS_GC
    /* Don't perform garbage collection if we still have room to grow. */
    if (allocation_size() < max_allocation_size)
        return;
#endif

    /*==========================================================*
     * TODO:  Implement mark-and-sweep garbage collection here! *
     *                                                          *
     * Mark all objects referenceable from either the global    *
     * environment, or from the evaluation stack.  Then sweep   *
     * through all allocated objects, freeing unmarked objects. *
     *==========================================================*/

    global_env = get_global_environment();
    eval_stack = get_eval_stack();

    /* ... TODO ... */

#ifndef ALWAYS_GC
    /* If we are still above the maximum allocation size, increase it. */
    if (allocation_size() > max_allocation_size) {
        max_allocation_size *= 2;

        printf("Increasing maximum allocation size to %ld bytes.\n",
            max_allocation_size);
    }
#endif

#ifdef GC_STATS
    vals_after = allocated_values.size;
    procs_after = allocated_lambdas.size;
    envs_after = allocated_environments.size;

    printf("GC Results:\n");
    printf("\tBefore: \t%d vals \t%d lambdas \t%d envs\n",
            vals_before, procs_before, envs_before);
    printf("\tAfter:  \t%d vals \t%d lambdas \t%d envs\n",
            vals_after, procs_after, envs_after);
    printf("\tChange: \t%d vals \t%d lambdas \t%d envs\n",
            vals_after - vals_before, procs_after - procs_before,
            envs_after - envs_before);
#endif
}

