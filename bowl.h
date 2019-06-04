/*
 * Please use the API to create and destroy objects as only this way
 * memory-safety and memory leaks can be guaranteed.
 *
 * With the API you can easily create structures like the following:
 *
 * root [list]
 *    child1 [list]
 *       key [literal] - "Username"
 *       value [literal] - "spacekookie"
 *    child2 [list]
 *       key [literal] - "Age"
 *       value [numerical] - 23
 *    child3
 *       subchild [list]
 *          ...
 *
 * Freeing the root node will free all children
 */

#ifndef _BOWL_H_
#define _BOWL_H_

#include <memory.h>
#include <stdbool.h>

/* A helpful macro that can take care of \0 termated strings! */
#define REAL_STRLEN(str) (strlen(str) + 1)

/* Also make sure we're _always_ interpreted as a C file */
#ifdef __cplusplus
extern "C" {
#endif


/* Type that determines what data is stored inside a tree-node */
typedef enum bowl_t {
    UNSET, LITERAL, NUMERIC, LONG_NUMERIC, BOOLEAN, LIST, PAIR, POINTER
} bowl_t;


struct bowl {
    bowl_t          type;
    short           encset;
    size_t          size, used;
    short           copy;
    union {
        char                *literal;
        bool                boolean;
        struct bowl        *(*list);
        void                *pointer;
#ifdef __LONG_LONG_SUPPORT__
        long long           numeral;
#else
        long                numeral;
#endif

    } payload;
};


/** Define some generic error codes first that we can propagate **/
typedef enum dt_err {

    /* General purpose error codes */
    FAILURE = -1,
    SUCCESS = 0,

    INVALID_PARAMS,         // A function didn't get the required parameters
    MALLOC_FAILED,          // A memory allocation failed
    INVALID_PAYLOAD,        // The payload of a node is invalid
    DATA_NOT_RELATED,       // Tried to split non-related trees
    NODE_NOT_FOUND,         // The sought after node was not found
    NODE_NOT_ORIGINAL,      // Tried to free a node which was a shallow copy
    QUERY_TOO_DEEP,

} dt_err;


/**
 * Malloc a new bowl object
 *
 * @param data Reference pointer to bowl element
 * @return
 */
dt_err bowl_malloc(struct bowl *(*data));


/**
 * Reset the type of a node and free child data
 *
 * @param data
 * @return
 */
dt_err bowl_resettype(struct bowl *data);


/**
 * Set the data element to a literal and save it's length
 *
 * @param data Reference to a bowl object
 * @param literal String to store
 * @return
 */
dt_err bowl_addliteral(struct bowl *data, const char *literal);


/**
 * Set the data element to a numeral
 *
 * @param data Reference to a bowl object
 * @param numeral Number to store
 * @return
 */
dt_err bowl_addnumeral(struct bowl *data, long numeral);


/**
 * Set the data element to a boolean. Do you really
 * need this? Consider using @bowl_add_numeral instead.
 *
 * @param data Reference to a bowl object
 * @param b boolean value (true, false)
 * @return
 */
dt_err bowl_addboolean(struct bowl *data, bool b);


/**
 * Add two new elements as a PAIR node under an existing node
 *
 * @param data bowl node to become the sub-root
 * @param key Reference pointer to the key node
 * @param value Reference pointer to the value node
 * @return
 */
dt_err bowl_addpair(struct bowl *data, struct bowl *(*key), struct bowl *(*value));


/**
 * Add a new data element to the resursive data store
 *
 * @param data Root reference
 * @param new_data Reference pointer to a new bowl node
 * @return
 */
dt_err bowl_addlist(struct bowl *data, struct bowl *(*new_data));


/**
 * This function enables you to store your own structures in a node. It however
 * also requires you to do some of your own memory management.
 *
 * WARNING: Can leak memory if pointer is previously set!
 *
 * To make sure that this function CAN NOT leak memory you should run
 * "bowl_resettype" on the root element to remove the pointer.
 *
 * Also make sure that no other part of your application will use the
 * pointer at a later date!
 *
 * @param data Root reference
 * @param ptr A pointer to store in this node
 * @return
 */
dt_err bowl_addpointer(struct bowl *data, void *ptr);


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
dt_err bowl_split_trees(struct bowl *data, struct bowl *sp);


/**
 * This function is very simmilar to  dt_err "bowl_addlist"
 * with the difference that it doesn't allocate new memory but instead
 * works with existing nodes.
 *
 * You need to provide a ROOT node which is of type list. It will
 * procede to add the second (merge) node into the child-list of the
 * root data node - essentially making them related.
 *
 * This allows for very efficient tree merging.
 *
 * @param data Root reference
 * @param merge Second root reference to merge
 * @return
 */
dt_err bowl_merge_trees(struct bowl *data, struct bowl *merge);


/**
 * You can use this function to search the structure of a root node to find the
 * parent of the node you provide as "data". It will leave the search pointer
 * blanked if the node can't be found in the structure.
 *
 * @param root Root reference to search
 * @param data The node we are searching for
 * @param parent The node parent we are interested in
 * @return
 */
dt_err bowl_parent(struct bowl *root, struct bowl *data, struct bowl **parent);


/**
 * Recursive tree search function that will return the first occurence match
 * to a provided payload (with an exact type). If you have data duplication
 * in your tree this _might_ return some false positives.
 *
 * @param data Root reference to search
 * @param found Empty pointer to put found node into
 * @param payload What should be found
 * @param type What type of data should be found
 * @return
 */
dt_err bowl_search_payload(struct bowl *data, struct bowl *(*found), void *payload, bowl_t type);


/**
 * Much like #{bowl_search_payload} but limiting it's search to keys in a list structure of certain depth.
 * This means that in a key-value store structure only top-level items can be searched or the entire
 * depth of the tree (or any vaue in between)
 *
 * @param data
 * @param found
 * @param payload
 * @param type
 * @return
 */
dt_err bowl_search_keypayload(struct bowl *data, struct bowl *(*found), void *payload, bowl_t type, int depth);


/**
 * Performs a deep copy of a data node hirarchy. Does not copy externally
 * pointed structures. Does garuantee safety of hirarchy.
 *
 * @param data
 * @param copy
 * @return
 */
dt_err bowl_copy_deep(struct bowl *data, struct bowl *(*copy));


/**
 * Performs a copy operation on a single node. Copies the payload on a pointer
 * level which means that strings and numbers will be duplicated whereas external
 * pointers and lists will only be references to the original content.
 *
 * Freeing the copy has no effect on the original payloads stored in other
 * nodes.
 *
 * @param data
 * @param copy
 * @return
 */
dt_err bowl_copy(struct bowl *data, struct bowl *(*copy));


/**
 * A retrieve function to get data back from a node that doesn't require
 * you to manually access parts of the struct.
 *
 * Needs to be provided with a reference to a pointer that can then be
 * written to. You can make the reference type specific if you know
 * what kind of data you're expecting. Please however note that compiler
 * errors might occur if you provide a wrong pointer type.
 *
 * @param data Node reference to access
 * @param val Reference pointer to write into
 * @return
 */
dt_err bowl_get(struct bowl *data, void *(*val));


/**
 * Return the type of a node as plain text
 *
 * @param data
 * @return
 */
const char *bowl_dtype(struct bowl *data);


/**
 * Prints the data bowl object and all of its children
 *
 * @param data
 */
void bowl_print(struct bowl *data);


/**
 * Will free the data reference and all of it's children. It will however NOT
 * touch pointers to objects that weren't allocated by libdyntree!
 *
 * @param data
 * @return
 */
dt_err bowl_free_shallow(struct bowl *data);


/**
 * Like #{bowl_free_shallow} but will also remove structs that
 * weren't allocated by libdyntree. Will throw warnings when trying
 * to free payloads from shallow copy nodes
 *
 * @param data
 * @return
 */
dt_err bowl_free(struct bowl *data);


/**************************
 *
 * Error handling functions
 *
 **************************/

const char *bowl_err_getmsg(dt_err *e);

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
dt_err bowl_encode_set(struct bowl *data, short setting);

/**
 * A simple list node walker that encodes a dyn_tree node hirarchy
 * into a json string. Requires the encoding setting to be set on the
 * root node in order to successfully encode.
 *
 * Can throw errors and initialise NULL return string.
 *
 * @param data
 * @param json_data
 * @return
 */
dt_err bowl_encode_json(struct bowl *data, char *json_data);


/**
 * Decodes a json string into a dyn_tree node hirarchy while providing
 * memory safety and error checking. Will gracefully return errors
 * if the provided json string is invalid or contains errors.
 *
 * @param data New root reference
 * @param json_data Input json string
 * @return
 */
dt_err bowl_decode_json(struct bowl *(*data), const char *json_data, size_t len);

#ifdef __cplusplus
}
#endif
#endif //_DYNTREE_H_
