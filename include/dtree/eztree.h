#ifndef LIBDYNTREE_EZTREE_H
#define LIBDYNTREE_EZTREE_H

#include <dtree/dtree.h>
#include <stdlib.h>

/* Also make sure we're _always_ interpreted as a C file */
#ifdef __cplusplus
extern "C" {
#endif

#define EZTREE_LITERAL   0xA
#define EZTREE_NUMERIC   0xB
#define EZTREE_NESTED    0xC


/**
 * An quick create function for a literal node
 *
 * @param string
 * @return
 */
dtree *eztree_new_literal(const char *string);


/**
 * A quick create function for a number (numeric) node
 * @param num
 * @return
 */
dtree *eztree_new_numeric(const long num);


/**
 * A quick create function for a string key and an arbitrary type value.
 * The value needs to be marked properly or errors might occur.
 *
 * Nested nodes can be passed as values but need to be declared in before. This means
 * that the tree needs to be built bottom-up.
 *
 * @param key
 * @param val
 * @param type
 * @return
 */
dtree *eztree_new_pair(const char *key, void *val, short type);


/**
 * A quick create function for a list node with a certain number of children
 * ready to go. Children will be placed into a bufer that needs to be provided
 * and the new parent will be returned from the function.
 *
 * Provided size needs to be size of the child-buffer or else errors might occur!
 *
 * @param list
 * @param size
 * @return
 */
dtree *eztree_new_list(dtree **list, size_t size);

#ifdef __cplusplus
}
#endif

#endif //LIBDYNTREE_EZTREE_H
