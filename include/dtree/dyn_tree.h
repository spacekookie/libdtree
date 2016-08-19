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

/* Type that determines what data is stored inside a tree-node */
typedef enum {
    UNSET, LITERAL, NUMERAL, RECURSIVE, PAIR, POINTER
} dt_uni_t;

typedef struct dtree {
    dt_uni_t        type;
    size_t          size, used;
    union {
        char                *literal;
        int                 numeral;
        struct dtree        *(*recursive);
        void                *pointer;
    } payload;
} dtree;


/**
 * Malloc a new dtree object
 *
 * @param data Reference pointer to dtree element
 * @return
 */
dt_err dtree_malloc(dtree *(*data));


/**
 * Reset the type of a node and free child data
 *
 * @param data
 * @return
 */
dt_err dtree_resettype(dtree *data);


/**
 * Set the data element to a literal and save it's length
 *
 * @param data Reference to a dtree object
 * @param literal String to store
 * @param length TRUE string length to use.
 * @return
 */
dt_err dtree_addliteral(dtree *data, const char *literal, size_t length);


/**
 * Set the data element to a numeral
 *
 * @param data Reference to a dtree object
 * @param numeral Number to store
 * @return
 */
dt_err dtree_addnumeral(dtree *data, int numeral);


/**
 * Add two new elements as a PAIR node under an existing node
 *
 * @param data dtree node to become the sub-root
 * @param key Reference pointer to the key node
 * @param value Reference pointer to the value node
 * @return
 */
dt_err dtree_addpair(dtree *data, dtree *(*key), dtree *(*value));


/**
 * Add a new data element to the resursive data store
 *
 * @param data Root reference
 * @param new_data Reference pointer to a new dtree node
 * @return
 */
dt_err dtree_addrecursive(dtree *data, dtree *(*new_data));


/**
 * This function enables you to store your own structures in a node. It however
 * also requires you to do some of your own memory management.
 *
 * WARNING: Can leak memory if pointer is previously set!
 *
 * To make sure that this function CAN NOT leak memory you should run
 * "dtree_resettype" on the root element to remove the pointer.
 *
 * Also make sure that no other part of your application will use the
 * pointer at a later date!
 *
 * @param data Root reference
 * @param ptr A pointer to store in this node
 * @return
 */
dt_err dtree_addpointer(dtree *data, void *ptr);


/**
 * A retrieve function to get data back from a node that doesn't require
 * you to manually access parts of the struct.
 *
 * Needs to be provided with a reference to a pointer that can then be
 * written to. You can make the reference type specific if you know
 * what kind of data you're expecting or leave it as a void* to let
 * libdyntree do the casting for you.
 *
 * @param data Node reference to access
 * @param val Reference pointer to write into
 * @return
 */
dt_err dtree_get(dtree *data, void *(*val));


/**
 * Return the type of a node as plain text
 *
 * @param data
 * @return
 */
const char *dtree_dtype(dtree *data);


/**
 * Prints the data dtree object and all of its children
 *
 * @param data
 */
void dtree_print(dtree *data);

/**
 * Will free the data reference and all of it's children. It will however NOT
 * touch pointers to objects that weren't allocated by libdyntree!
 *
 * @param data
 * @return
 */
dt_err dtree_free_shallow(dtree *data);

/**
 * Like #{dtree_free_shallow} but will also remove structs that
 * weren't allocated by libdyntree
 *
 * @param data
 * @return
 */
dt_err dtree_free(dtree *data);

#ifdef __cplusplus
}
#endif
#endif //_DYNTREE_H_
