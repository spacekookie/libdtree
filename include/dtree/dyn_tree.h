/*
 * Please use the API to create and destroy objects as only this way
 * memory-safety and memory leaks can be guaranteed.
 *
 * With the API you can easily create structures like the following:
 *
 * root [recursive]
 *    child1 [recursive]
 *       key [literal] - "Username"
 *       value [literal] - "spacekookie"
 *    child2 [recursive]
 *       key [literal] - "Age"
 *       value [numerical] - 23
 *    child3
 *       subchild [recursive]
 *          ...
 *
 * Freeing the root node will free all children
 */

#ifndef _DYNTREE_H_
#define _DYNTREE_H_

#include "dyn_err.h"
#include <memory.h>


/* Also make sure we're _always_ interpreted as a C file */
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UNSET, LITERAL, NUMERAL, RECURSIVE, PAIR
} dt_uni_t;

typedef struct dtree {
    dt_uni_t        type;
    size_t          size, used;
    union {
        char                *literal;
        int                 numeral;
        struct dtree     *(*recursive);
    } payload;
} dtree;

/** Malloc a new dtree object */
dt_err dtree_malloc(dtree *(*data));

dt_err dtree_resettype(dtree *data);

/** Set the data element to a literal and save it's length */
dt_err dtree_addliteral(dtree *data, const char *literal, size_t length);

/** Set the data element to a numeral */
dt_err dtree_addnumeral(dtree *data, int numeral);

/** Add two new elements as a PAIR node under an existing node */
dt_err dtree_addpair(dtree *data, dtree *(*key), dtree *(*value));

/** Add a new data element to the resursive data store */
dt_err dtree_addrecursive(dtree *data, dtree *(*new_data));

dt_err dtree_get(dtree *data, void *(*val));

const char *dtree_dtype(dt_uni_t type);

/** Prints*/
void dtree_print(dtree *data);

/** Will free all memory allocated by this element and it's children */
dt_err dtree_free(dtree *data);

#ifdef __cplusplus
}
#endif
#endif //_DYNTREE_H_
