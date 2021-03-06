#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#include "cache.h"


/* Set this to a nonzero value and rebuild to see debug output. */
#define DEBUG_CACHE 0


/* Set this to 0 to activate your custom replacement policy, which is
 * hopefully smarter and better than a random replacement policy!
 */
#define RANDOM_REPLACEMENT_POLICY 0


/* Local functions used by the cache implementation, roughly in order of
 * usage.
 */

unsigned char cache_read_byte(membase_t *mb, addr_t address);
void cache_write_byte(membase_t *mb, addr_t address, unsigned char value);
void cache_free(membase_t *mb);

void cache_print_stats(membase_t *mb);
void cache_reset_stats(membase_t *mb);

cacheline_t *resolve_cache_access(cache_t *p_cache, addr_t address);

void decompose_address(cache_t *p_cache, addr_t address,
    addr_t *tag, addr_t *set, addr_t *offset);

addr_t get_block_start_from_address(cache_t *p_cache, addr_t address);
addr_t get_offset_in_block(cache_t *p_cache, addr_t address);
addr_t get_block_start_from_line_info(cache_t *p_cache,
                                      addr_t tag, addr_t set_no);

cacheline_t * find_line_in_set(cacheset_t *p_set, addr_t tag);

cacheline_t * choose_victim(cacheset_t *p_set);
cacheline_t * evict_cache_line(cache_t *p_cache, cacheset_t *p_set);

void load_cache_line(cache_t *p_cache, cacheline_t *p_line, addr_t address,
                     addr_t tag);
void write_back_cache_line(cache_t *p_cache, cacheline_t *p_line,
                           addr_t set_no);


/* Initializes the members of the cache_t struct to be a cache with the
 * specified block size, number of cache-sets, and the number of cache lines
 * per set.  This requires a number of heap allocations, so the allocated
 * memory must be released when cleaning up the cache.
 */
void init_cache(cache_t *p_cache, unsigned int block_size,
                unsigned int num_sets, unsigned int lines_per_set,
                membase_t *next_mem) {
    addr_t set_no;
    unsigned int line_no;

    assert(p_cache != NULL);
    assert(next_mem != NULL);

    assert(is_power_of_2(block_size));
    assert(is_power_of_2(num_sets));

    bzero(p_cache, sizeof(cache_t));

    /* This is the memory to go to if the cache can't satisfy a request. */
    p_cache->next_memory = next_mem;

    /* Set up the functions this cache exposes. */
    p_cache->read_byte = cache_read_byte;
    p_cache->write_byte = cache_write_byte;
    p_cache->print_stats = cache_print_stats;
    p_cache->reset_stats = cache_reset_stats;
    p_cache->free = cache_free;

    /* These are various parameters for the cache. */

    p_cache->block_size = block_size;
    p_cache->num_sets = num_sets;
    p_cache->cache_sets = malloc(num_sets * sizeof(cacheset_t));

    p_cache->sets_addr_bits = log_2(num_sets);
    p_cache->block_offset_bits = log_2(block_size);

    /* The remaining code initializes each cache set and the lines in
     * each set.
     */

    for (set_no = 0; set_no < num_sets; set_no++) {
        /* Get a pointer to the specific cache set to initialize. */
        cacheset_t *p_set = p_cache->cache_sets + set_no;

        p_set->set_no = set_no;
        p_set->num_lines = lines_per_set;
        p_set->cache_lines = malloc(lines_per_set * sizeof(cacheline_t));

        for (line_no = 0; line_no < lines_per_set; line_no++) {
            cacheline_t *p_line = p_set->cache_lines + line_no;
            bzero(p_line, sizeof(cacheline_t));

            p_line->line_no = line_no;
            p_line->block = malloc(block_size);
        }
    }
}


/* This function implements reading bytes of memory through the cache. */
unsigned char cache_read_byte(membase_t *mb, addr_t address) {
    cache_t *p_cache = (cache_t *) mb;
    cacheline_t *p_line;
    addr_t block_offset;

#if DEBUG_CACHE
    printf("Resolving cache read to address %u\n", address);
#endif

    p_line = resolve_cache_access(p_cache, address);
    /* Update the access_time of the cache line, b/c we are reading from it */
    p_line->access_time = clock_tick();
    block_offset = get_offset_in_block(p_cache, address);

#if DEBUG_CACHE
    printf(" * Block offset within cache line:  %u\n", block_offset);
#endif

    /* Return the byte read by the requester. */
    p_cache->num_reads++;
    return p_line->block[block_offset];
}


/* This function implements writing bytes of memory through the cache. */
void cache_write_byte(membase_t *mb, addr_t address, unsigned char value) {
    cache_t *p_cache = (cache_t *) mb;
    cacheline_t *p_line = resolve_cache_access(p_cache, address);
    /* Update the access_time of the cache line, b/c we are reading from it */
    p_line->access_time = clock_tick();
    addr_t block_offset = get_offset_in_block(p_cache, address);

    /* Write the byte specified by the requester. */
    p_cache->num_writes++;
    p_line->block[block_offset] = value;
    p_line->dirty = 1;
}


/* This function prints the statistics for the cache itself, and then calls
 * the next level of the memory to print its statistics.
 */
void cache_print_stats(membase_t *mb) {
    cache_t *p_cache = (cache_t *) mb;

    double miss_rate = (double) p_cache->num_misses;
    miss_rate /= (double) (p_cache->num_hits + p_cache->num_misses);
    miss_rate *= 100;

    printf(" * Cache reads=%lld writes=%lld hits=%lld misses=%lld "
           "\n", p_cache->num_reads, p_cache->num_writes,
           p_cache->num_hits, p_cache->num_misses);
    printf("   miss-rate=%.2f%% %s replacement policy\n", miss_rate,
           (RANDOM_REPLACEMENT_POLICY ? "random" : "LRU"));

    p_cache->next_memory->print_stats(p_cache->next_memory);
}


/* This method resets the statistics for the cache, and passes the operation
 * on to the next level of the cache as well.
 */
void cache_reset_stats(membase_t *mb) {
    cache_t *p_cache = (cache_t *) mb;

    p_cache->num_reads = 0;
    p_cache->num_writes = 0;
    p_cache->num_hits = 0;
    p_cache->num_misses = 0;

    p_cache->next_memory->reset_stats(p_cache->next_memory);
}


/* This method frees all heap-allocated memory used by the simulated cache.
 * The method does *not* pass the call on to the next level of the memory.
 */
void cache_free(membase_t *mb) {
    cache_t *p_cache = (cache_t *) mb;
    int i_set, i_line;

    for (i_set = 0; i_set < p_cache->num_sets; i_set++) {
        cacheset_t *p_set = p_cache->cache_sets + i_set;
        for (i_line = 0; i_line < p_set->num_lines; i_line++) {
            cacheline_t *p_line = p_set->cache_lines + i_line;
            free(p_line->block);
        }
        free(p_set->cache_lines);
    }
    free(p_cache->cache_sets);
}


/* This method flushes lines out of the cache so that all modified data in the
 * cache is properly reflected in the next level of the simulated memory.
 */
int flush_cache(cache_t *p_cache) {
    addr_t i_set, i_line;
    int flushed;

    flushed = 0;
    for (i_set = 0; i_set < p_cache->num_sets; i_set++) {
        cacheset_t *p_set = p_cache->cache_sets + i_set;
        for (i_line = 0; i_line < p_set->num_lines; i_line++) {
            cacheline_t *p_line = p_set->cache_lines + i_line;
            if (p_line->valid && p_line->dirty) {
                write_back_cache_line(p_cache, p_line, i_set);
                flushed++;
            }
        }
    }

    return flushed;
}


/*---------------------------------------------------------------------------
 * CACHE HELPER FUNCTIONS
 */


/* This function is used by both the read-byte and write-byte functions to
 * ensure that the cache contains a cache-line for the specified address.
 * This way, the read or write can be performed against the cache-line.  If
 * the cache doesn't contain a line for the specified address, the
 * corresponding block will be loaded from the next level of the memory.  An
 * eviction will also occur if the cache doesn't currently have room for the
 * new line.
 */
cacheline_t *resolve_cache_access(cache_t *p_cache, addr_t address) {
    addr_t tag, set_no, block_offset;
    cacheset_t *p_set;
    cacheline_t *p_line;

    /* Map the address to a cache set, and pull out the tag and block
     * offset too.
     */
    decompose_address(p_cache, address, &tag, &set_no, &block_offset);
#if DEBUG_CACHE
    printf(" * Decomposed address %u into tag %u, set %u, and offset %u\n",
           address, tag, set_no, block_offset);
#endif

    /* Get the cache set that should contain the address. */
    p_set = p_cache->cache_sets + set_no;
    p_line = find_line_in_set(p_set, tag);

    if (p_line == NULL) {
        /* CACHE MISS.  :-( */
        p_cache->num_misses++;

#if DEBUG_CACHE
        printf(" * Cache miss.\n");
#endif

        /* Resolve the cache miss. */
        p_line = evict_cache_line(p_cache, p_set);
        load_cache_line(p_cache, p_line, address, tag);
    }
    else {
        /* CACHE HIT!  :-) */
        p_cache->num_hits++;
    }

    return p_line;
}


/* This function takes a cache and an address being accessed through the
 * cache, and it breaks the address down into the values needed by the cache:
 *  - The tag that identifies the block.
 *  - The cache set that the block maps to.
 *  - The offset within the block corresponding to the address.
 *
 * Since the function returns multiple values, the results are returned in
 * out-parameters tag, set, and offset, which are pointers to the locations
 * where the actual values should be stored.
 */
void decompose_address(cache_t *p_cache, addr_t address,
    addr_t *tag, addr_t *set, addr_t *offset) {

    assert(p_cache != NULL);
    assert(tag != NULL);
    assert(set != NULL);
    assert(offset != NULL);

    /*
     * === Number of bits for each value ===
     * offset: block_offset_bits = b
     * set: sets_addr_bits = s
     * tag: 32 - block_offset_bits - sets_addr_bits = t
     * (addr_t is an unsigned int)
     */

    unsigned int b = p_cache->block_offset_bits;
    unsigned int s = p_cache->sets_addr_bits;
    unsigned int t = sizeof(addr_t) * 8 - s - b; // 8 bits per byte

    // So to get bottom b bits, we need a bitmask. Consider the following:
    // 2^5 - 1 = 0b11111. More generally, 2^x - 1 = 0b111...111, where there
    // are x 1's. So this is good; we can use 2^b - 1 as our bitmask.
    int offset_bitmask = (1 << b) - 1; // = 2^b - 1
    *offset = address & offset_bitmask;

    // Now shift address down by b bits so that the s bits are at the bottom
    address >>= b;
    // Create another bitmask, similarly to before
    int set_bitmask = (1 << s) - 1;    // = 2^s - 1
    *set = address & set_bitmask;

    // Now shift address down by s bits so that the t bits are at the bottom
    address >>= s;
    // Create yet another bitmask, similarly to before
    int tag_bitmask = (1 << t) - 1;    // = 2^t - 1
    *tag = address & tag_bitmask;
}


/* This function takes a cache and an address being accessed through the
 * cache, and returns the address of the start of the block that the access
 * falls within.  It is used for loading cache lines from the next level of
 * the memory.
 */
addr_t get_block_start_from_address(cache_t *p_cache, addr_t address) {
    // Get corresponding block_num of the passed-in address.
    int block_num = address / p_cache->block_size;
    // Turn this back into an address by multiplying by block_size.
    addr_t block_start_addr = block_num * p_cache->block_size;
    return block_start_addr;
}


/* This function takes a cache and an address being accessed through the
 * cache, and returns the offset within the block that the access occurs at.
 */
addr_t get_offset_in_block(cache_t *p_cache, addr_t address) {
    /* To get offset, decompose the address and return the output param */
    addr_t tag;
    addr_t set;
    addr_t offset;
    decompose_address(p_cache, address, &tag, &set, &offset);
    return offset;
}


/* This function takes a cache, and the set-number and tag of a given cache
 * line, and it computes the starting address of the memory block that
 * corresponds with this cache line.  This is used for writing back cache
 * lines to the next level of the memory.
 */
addr_t get_block_start_from_line_info(cache_t *p_cache,
                                      addr_t tag, addr_t set_no) {
    /*
     * Need to map this information back to address in main memory. Do so
     * by reconstructing the address and calling get_block_start_from_address.
     */
    /* Helper variables */
    unsigned int b = p_cache->block_offset_bits;
    unsigned int s = p_cache->sets_addr_bits;
    unsigned int t = sizeof(addr_t) * 8 - s - b; // 8 bits per byte

    /* Reconstruct address */
    addr_t address = tag;
    address <<= s;
    address |= set_no;
    address <<= b;
    addr_t block_start = get_block_start_from_address(p_cache, address);
    return block_start;
}


/* This function searches through a cache set, looking for the cache line with
 * the specified tag.  If no line can be found with this tag, the function
 * returns NULL.
 */
cacheline_t * find_line_in_set(cacheset_t *p_set, addr_t tag) {
#if DEBUG_CACHE
    printf(" * Finding line with tag %u in cache set:\n", tag);
#endif

    int j;
    char valid;
    addr_t curr_tag;
    cacheline_t *cache_lines_temp = p_set->cache_lines;
    /* Loop through the cache lines, search for the desired one */
    for (j = 0; j < p_set->num_lines; ++j, ++cache_lines_temp) {
        curr_tag = (*cache_lines_temp).tag;
        valid = (*cache_lines_temp).valid;
        // This is the cache line we are looking for
        if (curr_tag == tag && valid == 1) {
#if DEBUG_CACHE
            printf("Found the desired cache line\n");
#endif
            return &(*cache_lines_temp);
        }
    }

#if DEBUG_CACHE
    printf("Desired cache line was not found; returning 0\n");
#endif
    return NULL;
}


/* This function chooses a victim cache-line to evict, when a new cache line
 * must be loaded into the cache.  Note that this function is slightly mis-
 * named; if it selects a cache line that is currently invalid, nothing will
 * actually be evicted; the line will simply be used to store the new block
 * of data.
 */
cacheline_t * choose_victim(cacheset_t *p_set) {
    cacheline_t *victim = NULL;
    int i_victim;

#if RANDOM_REPLACEMENT_POLICY
    /* Randomly choose a victim line to evict. */
    i_victim = rand() % p_set->num_lines;
    victim = p_set->cache_lines + i_victim;
#else
    /* TODO:  Implement the LRU eviction policy. */
    int j;
    char valid;
    unsigned long long int min_access_time = LONG_MAX;
    unsigned long long int curr_access_time;
    cacheline_t *cache_lines_temp = p_set->cache_lines;
    /* Loop through the cache lines, search for the desired one */
    for (j = 0; j < p_set->num_lines; ++j, ++cache_lines_temp) {
        valid = (*cache_lines_temp).valid;
        // If we come across a line that is invalid, just return it because it's
        // currently empty
        if (valid == 0) {
#if DEBUG_CACHE
            printf("Found an invalid cache line; return immediately because \
                    it is currently empty\n");
#endif
            victim = &(*cache_lines_temp);
            break;
        }
        // Otherwise, find the line with the smallest access-time value (LRU)
        curr_access_time = (*cache_lines_temp).access_time;
        if (curr_access_time < min_access_time) {
            min_access_time = curr_access_time;
            victim = &(*cache_lines_temp);
        }
    }

#endif

#if DEBUG_CACHE
    if (victim->valid) {
        printf(" * Chose victim line to evict:  tag %u, set %u\n",
               victim->tag, p_set->set_no);
    }
#endif

    return victim;
}


/* This function handles the case when space must be made in the current
 * cache set.  A victim is selected using the choose_victim() helper, and if
 * it is dirty, this function also ensures that the cache line is written
 * back to the next level of the memory.  At completion, the function returns
 * a pointer to the newly emptied and invalidated cache line that can be used
 * to load a new block from the next level of memory.
 */
cacheline_t * evict_cache_line(cache_t *p_cache, cacheset_t *p_set) {
    /* Randomly choose a victim line to evict. */
    cacheline_t *victim = choose_victim(p_set);

    if (victim->valid && victim->dirty) {
        /* The line being evicted is dirty, so we need to
         * write it back to the next level.
         */

#if DEBUG_CACHE
        printf(" * Victim cache line is dirty; writing back.\n");
#endif

        write_back_cache_line(p_cache, victim, p_set->set_no);
    }

    victim->valid = 0;
    victim->dirty = 0;
    victim->tag = 0;

    return victim;
}


/* This function loads a block of data from the next level of the memory into
 * the specified cache-line of this cache.  The tag could be computed from
 * the address, but it is passed in as an argument since it was already
 * computed earlier on.
 */
void load_cache_line(cache_t *p_cache, cacheline_t *p_line, addr_t address,
                     addr_t tag) {
    membase_t *next_mem = p_cache->next_memory;
    addr_t start_addr;
    int offset;

    /* Determine the start of the block that holds the specified address. */
    start_addr = get_block_start_from_address(p_cache, address);

    /* Read each byte of the new line from the next level. */
    for (offset = 0; offset < p_cache->block_size; offset++)
        p_line->block[offset] = read_byte(next_mem, start_addr + offset);

    p_line->valid = 1;
    p_line->dirty = 0;
    p_line->tag = tag;
}


/* This function writes a block of dirty data from the specified cache-line
 * into the next level of the memory.  The tag and set-number must be used to
 * compute the starting address of the block, since the address itself is not
 * stored in the cache line.
 */
void write_back_cache_line(cache_t *p_cache, cacheline_t *p_line,
                           addr_t set_no) {
    /* The line being evicted is dirty, so we need to
     * write it back to the next level.
     */
    membase_t *next_mem = p_cache->next_memory;
    addr_t start_addr;
    int offset;

    assert(p_line->valid);
    assert(p_line->dirty);

#if DEBUG_CACHE
    printf(" * Tag of cache line being written back is %u\n", p_line->tag);
#endif

    /* Reconstruct the address where the block is stored, so we can
     * write it back to the next level.
     */
    start_addr = get_block_start_from_line_info(p_cache, p_line->tag, set_no);

#if DEBUG_CACHE
    printf(" * Start address of cache line being written back is %u\n",
           start_addr);
#endif

    /* Write each byte of the victim line out to the next level. */
    for (offset = 0; offset < p_cache->block_size; offset++)
        write_byte(next_mem, start_addr + offset, p_line->block[offset]);
}
