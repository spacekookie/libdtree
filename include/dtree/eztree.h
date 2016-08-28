//
// Created by spacekookie on 28/08/16.
//

#ifndef LIBDYNTREE_EZTREE_H
#define LIBDYNTREE_EZTREE_H

#include <dtree/dtree.h>
#include <stdlib.h>

/**
 * Shortcut function that allocates a new string node. Beware however:
 *
 * THIS FUNCTION DOES NOT RETURN WARNINGS OR ERROR CODES!
 *
 * @param string
 * @return
 */
static dtree *dtree_alloc_literal(const char *string)
{
    dtree *node;
    dtree_malloc(&node);
    dtree_addliteral(node, string);
    return node;
}

/**
 * Shortcut function that allocates a new numerical node.
 * Beware however:
 *
 * THIS FUNCTION DOES NOT RETURN WARNINGS OR ERROR CODES!
 *
 * @param num
 * @return
 */
static dtree *dtree_alloc_numeral(const long num)
{
    dtree *node;
    dtree_malloc(&node);
    dtree_addnumeral(node, num);
    return node;
}

/**
 * Shortcut function which creates two nodes as pair under a root
 * node which is returned. Beware however:
 *
 * THIS FUNCTION DOES NOT RETURN WARNINGS OR ERROR CODES!
 *
   @param key Will be the key node
 * @param val Will be the value node
 * @return New root node with key-val children
 */
static dtree *dtree_allocpair_new(dtree **key, dtree **val)
{
    dtree *root;
    dtree_malloc(&root);
    dtree_addpair(root, key, val);
    return root;
}


/**
 * Shortcut function which allocates a list of nodes in a list under
 * a root node listly.
 *
 * WARNING: Return value is allocated on heap. MUST FREE MANUALLY!
 * WARNING: THIS FUNCTION DOES NOT RETURN WARNINGS OR ERROR CODES!
 *
 * @param root
 * @param count
 * @return
 */
static dtree **dtree_alloc_listlist(dtree **root, unsigned int count)
{
    dtree **nodes = malloc(sizeof(dtree**) * count);

    dtree_malloc(root);

    int i;
    for(i = 0; i < count; i++)
        dtree_addlist(*root, &nodes[i]);

    return nodes;
}

#endif //LIBDYNTREE_EZTREE_H
