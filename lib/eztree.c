// Include eztree header file
#include <dtree/eztree.h>

dtree *eztree_new_literal(const char *string)
{
    dtree *node;
    dtree_malloc(&node);
    dtree_addliteral(node, string);
    return node;
}


dtree *eztree_new_numeric(const long num)
{
    dtree *node;
    dtree_malloc(&node);
    dtree_addnumeral(node, num);
    return node;
}


dtree *eztree_new_pair(const char *kdata, void *vdata, short type)
{
    dtree *root, *key, *val;

    /* Allocate nodes */
    dtree_malloc(&root);
    dtree_addpair(root, &key, &val);

    /* Fill the data */
    dtree_addliteral(key, kdata);
    switch(type) {
        case EZTREE_LITERAL:
            dtree_addliteral(val, (char*) vdata);
            break;

        case EZTREE_NUMERIC:
            // FIXME: This might be dangerous on 32bit
            dtree_addnumeral(val, (long) vdata);
            break;

        case EZTREE_NESTED:
        {
            dtree *tmp;
            dtree_addlist(val, &tmp);

            /* Manually override data */
            memcpy(val->payload.list[0], vdata, sizeof(vdata));
            break;
        }

        default: break;
    }

    return root;
}

//
//dtree *eztree_new_list(dtree **list, size_t size)
//{
//    /* Prepare our buffer */
//    memset(list, 0, sizeof(dtree*) * size);
//
//    /* Prepare root node */
//    dtree *root;
//    dtree_malloc(&root);
//
//    /* Add apropriate number of children to root node */
//    int i;
//    for(i = 0; i < size; i++) {
//        dtree *tmp;
//        dtree_addlist(root, &tmp);
//        list[i] = tmp;
//    }
//
//    return root;
//}