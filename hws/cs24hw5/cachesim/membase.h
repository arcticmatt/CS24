#ifndef MEMBASE_H
#define MEMBASE_H


/* This typedef specifies the type we use for "addresses" in the memory
 * simulation.
 */
typedef unsigned int addr_t;


/* This struct defines the basic operations that must be present in all of our
 * "memory" types.  The memory_t and cache_t types both have *exactly* the
 * same initial set of members, so that a pointer to a cache_t or memory_t
 * struct can be case to a membase_t pointer.  (Obviously, the cache_t and
 * memory_t types add other members *after* these members as well.)
 */
typedef struct membase_t {
    /* The number of reads that occurred at this level of the memory. */
    unsigned long long num_reads;

    /* The number of writes that occurred at this level of the memory. */
    unsigned long long num_writes;

    /* The function to read a byte from the memory. */
    unsigned char (*read_byte)(struct membase_t *mb, addr_t address);

    /* The function to write a byte to the memory. */
    void (*write_byte)(struct membase_t *mb, addr_t address, unsigned char value);

    /* The function to print the memory's access statistics. */
    void (*print_stats)(struct membase_t *mb);

    /* The function to reset the memory's access statistics. */
    void (*reset_stats)(struct membase_t *mb);

    /* The function to release any internally allocated data used by
     * the memory.
     */
    void (*free)(struct membase_t *mb);

} membase_t;


/* Returns nonzero if the input is a power of 2, or zero otherwise. */
unsigned int is_power_of_2(unsigned int n);

/* For a given n that is a power of 2, where 2^m = n, this function
 * returns m.
 */
unsigned int log_2(unsigned int n);


unsigned char read_byte(membase_t *mb, addr_t address);
void write_byte(membase_t *mb, addr_t address, unsigned char value);


/*
 * These functions expose the memory as an array of signed integers or floats,
 * instead of an array of bytes.  Each index references a 4-byte value; the
 * index is multiplied by 4 when accessing the underlying memory.  Values are
 * stored in little-endian order, as traditional for IA32.
 */

int read_int(membase_t *mb, unsigned int index);
void write_int(membase_t *mb, unsigned int index, int value);

float read_float(membase_t *mb, unsigned int index);
void write_float(membase_t *mb, unsigned int index, float value);


/* This function can be used to emulate a hardware clock e.g. for tagging cache
 * lines in order to implement an LRU replacement policy when evicting cache
 * lines.
 */
unsigned long long int clock_tick();


#endif /* MEMBASE_H */
