/*============================================================================
 * Implementation of the RANDOM page replacement policy.
 *
 * We don't mind if policies use malloc() and free(), just because it keeps
 * things simpler.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "vmpolicy.h"


/*============================================================================
 * Page-List Data Structure
 */


/* A single node in the page-info linked list. */
typedef struct pageinfo_t {
    page_t page;
    struct pageinfo_t *next;
} pageinfo_t;


/* A page-info linked list structure for tracking page details. */
typedef struct pagelist_t {
    pageinfo_t *head;
    pageinfo_t *tail;
} pagelist_t;


/* Find the pageinfo struct with the specified page number.  NULL is returned
 * if the page cannot be found.  If desired, the previous page can be returned
 * by this function as well.
 */
pageinfo_t * find_page(pagelist_t *list, page_t page, pageinfo_t **p_prev) {
    pageinfo_t *pginfo, *prev;

    assert(list != NULL);

    pginfo = list->head;
    prev = NULL;
    while (pginfo != NULL) {
        if (pginfo->page == page) {
            if (p_prev != NULL)
                *p_prev = prev;
            return pginfo;
        }
        prev = pginfo;
        pginfo = pginfo->next;
    }
    return NULL;
}


/* Add a pageinfo_t struct to the tail of the specified list. */
void add_to_tail(pagelist_t *list, pageinfo_t *pginfo) {
    assert(list != NULL);
    assert(pginfo != NULL);

    pginfo->next = NULL;

    if (list->tail != NULL)
        list->tail->next = pginfo;
    else
        list->head = pginfo;

    list->tail = pginfo;
}


/* Remove a pageinfo_t struct from the specified list.  The previous-node
 * pointer is necessary so that the linked list can be updated properly.  If
 * the node being removed is the first node in the list, the prev pointer
 * should be NULL (obvious).
 */
void remove_from_list(pagelist_t *list, pageinfo_t *pginfo, pageinfo_t *prev) {
    assert(prev == NULL || prev->next == pginfo);

    if (prev != NULL) {
        prev->next = pginfo->next;
    }
    else {
        assert(list->head == pginfo);
        list->head = pginfo->next;
    }

    if (pginfo == list->tail)
        list->tail = prev;

    pginfo->next = NULL;
}


/* Add the specified page to the end of the linked list.  This is a wrapper
 * for the add_to_tail() function.
 */
void add_page(pagelist_t *list, page_t page) {
    pageinfo_t *pginfo;

    assert(list != NULL);
    assert(find_page(list, page, NULL) == NULL);

    /* Always add the page to the back of the queue. */

    pginfo = malloc(sizeof(pageinfo_t));
    pginfo->page = page;
    pginfo->next = NULL;

    add_to_tail(list, pginfo);
}


/*============================================================================
 * Policy Implementation
 */


/* The list of pages that are currently resident. */
static pagelist_t pagelist;


/* Initialize the policy.  Return 0 for success, -1 for failure. */
int policy_init() {
    fprintf(stderr, "Using CLRU eviction policy.\n\n");
    pagelist.head = NULL;
    pagelist.tail = NULL;
    return 0;
}


/* This function is called when the virtual memory system maps a page into the
 * virtual address space.  Record that the page is now resident.
 */
void policy_page_mapped(page_t page) {
    add_page(&pagelist, page);
}


/* This function is called when the virtual memory system unmaps a page from
 * the virtual address space.  Remove the page from the list of resident
 * pages.
 */
void policy_page_unmapped(page_t page) {
    pageinfo_t *pginfo, *prev;

    pginfo = find_page(&pagelist, page, &prev);
    assert(pginfo != NULL);

    remove_from_list(&pagelist, pginfo, prev);
}


/* This function is called when the virtual memory system has a timer tick. */
void policy_timer_tick() {
    pageinfo_t *pginfo, *prev;

    pginfo = pagelist.head;
    prev = NULL;

    /*
     * Loop through all resident pages in the queue. If a page has been
     * accessed, clear the access bit and move it to the back of the queue.
     *
     * Else, just update the pointers.
     */
    while (pginfo != NULL) {
        if (is_page_accessed(pginfo->page)) {
            // Clear access bit
            clear_page_accessed(pginfo->page);
            // Set page permission so we can set the access bit again
            set_page_permission(pginfo->page, PAGEPERM_NONE);
            // Update next pointer of previous pginfo
            if (prev != NULL) {
                prev->next = pginfo->next;
            }
            /* Move current pginfo to tail and update pginfo pointer */
            assert(pagelist.tail != NULL); // Since we are in the while loop...
            if (pginfo != pagelist.tail) {
                pageinfo_t *next = pginfo->next;
                // If we are moving the previous head to the tail, update
                // pagelist.head
                if (prev == NULL) {
                    pagelist.head = next;
                }
                pginfo->next = NULL;
                pagelist.tail->next = pginfo;
                pagelist.tail = pginfo;
                pginfo = next;
            }
        } else {
            prev = pginfo;
            pginfo = pginfo->next;
        }
    }
}


/* Choose a page from the front of the queue (the linked list) to evict. This
 * is very simple, since we keep track of the head of the linked list.
 *
 * Since we keep "recently" accessed pages towards the back of the queue
 * (this is implemented in policy_timer_tick), taking a page from the front
 * of the queue will usually evict a page that hasn't been accessed very
 * recently.
 *
 * This, in effect, gives us a CLRU page-replacement policy.
 */
page_t choose_victim_page() {
    assert(pagelist.head != NULL);

    page_t victim = pagelist.head->page;

#if VERBOSE
    fprintf(stderr, "Choosing victim page %u to evict.\n", victim);
#endif

    return victim;
}
