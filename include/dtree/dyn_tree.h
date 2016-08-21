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

#include <memory.h>

/* A helpful macro that can take care of \0 termated strings! */
#define REAL_STRLEN(str) (strlen(str) + 1)

#define DYNTREE_ENCODE_NONE        0x0
#define DYNTREE_JSON_MINIFIED      0xA
#define DYNTREE_JSON_HUMAN         0xB

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
    short           encset;
} dtree;


/** Define some generic error codes first that we can propagate **/
typedef enum dt_err {

    /* General purpose error codes */
            FAILURE = -1,
    SUCCESS = 0,

    INVALID_PARAMS,
    MALLOC_FAILED,
    INVALID_PAYLOAD

} dt_err;


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
 * This function takes two nodes as arguments. The nodes MUST be
 * related or an error will be thrown. Both nodes will still
 * be accessable after this operation but no longer be related to
 * each other.
 *
 * The second node will be removed from the tree of the root node.
 *
 *
 *
 * @param data Root reference
 * @param sp Subtree node related to root to split off
 * @return
 */
dt_err dtree_split_trees(dtree *data, dtree *sp);

/**
 * This function is very simmilar to  dt_err "dtree_addrecursive"
 * with the difference that it doesn't allocate new memory but instead
 * works with existing nodes.
 *
 * You need to provide a ROOT node which is of type recursive. It will
 * procede to add the second (merge) node into the child-list of the
 * root data node - essentially making them related.
 *
 * This allows for very efficient tree merging.
 *
 * @param data Root reference
 * @param merge Second root reference to merge
 * @return
 */
dt_err dtree_merge_trees(dtree *data, dtree *merge);

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

/**************************
 *
 * Error handling functions
 *
 **************************/

const char *dtree_err_getmsg(dt_err *e);

/***************************
 *
 * Encoding/ Decoding support hooks
 *
 ***************************/

/**
 * This function sets the wanted encoding setting on a
 * root node (and assumes for children). Without setting flags via
 * this function first the encode will fail.
 *
 * @param data Root reference
 * @param setting Look at DYNTREE_JSON flags for options
 * @return
 */
dt_err dtree_encode_set(dtree *data, short setting);

/**
 * A simple recursive node walker that encodes a dyn_tree node hirarchy
 * into a json string. Requires the encoding setting to be set on the
 * root node in order to successfully encode.
 *
 * Can throw errors and initialise NULL return string.
 *
 * @param data
 * @param json_data
 * @return
 */
dt_err dtree_encode_json(dtree *data, char *(*json_data));


/**
 * Decodes a json string into a dyn_tree node hirarchy while providing
 * memory safety and error checking. Will gracefully return errors
 * if the provided json string is invalid or contains errors.
 *
 * @param data New root reference
 * @param json_data Input json string
 * @return
 */
dt_err dtree_decode_json(d_tree *(*data), const char *json_data);


#ifdef __cplusplus
}
#endif
#endif //_DYNTREE_H_
