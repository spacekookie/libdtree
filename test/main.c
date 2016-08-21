
#include <stdio.h>
#include <dtree/dyn_tree.h>

/**
 * A small test that creates a tree, splits the nodes
 * and then merges them again.
 */
dt_err split_and_merge();

dt_err search_for_payload();

#define TEST(function) \
    printf("Running '%s'...", #function); \
    fflush(stdout); \
    err = function(); \
    printf(" %s\n", (err == 0) ? "OK!" : "FAILED!"); \
    if(err) goto end;

int main(void)
{
    dt_err err;
    printf("=== libdyntree test suite ===\n");

    /* Search inside trees */
    TEST(search_for_payload)

    /* Split and merge trees */
    TEST(split_and_merge)

    end:
    printf("==== done ====\n");
    return err;
}


/*************** TEST IMPLEMENTATIONS ****************/

dt_err split_and_merge()
{
    dt_err err;

    /* Allocate a node named root */
    dtree *root;
    err = dtree_malloc(&root);
    if(err) goto exit;

    /* Add child as a recursive node to root */
    dtree *child;
    err = dtree_addrecursive(root, &child);
    if(err) goto exit;

    /* Make child a literal node containing the works of shakespeare */
    const char *hamlet = "To be, or not to be: that is the question:\n"
            "Whether 'tis nobler in the mind to suffer\n"
            "The slings and arrows of outrageous fortune,\n"
            "Or to take arms against a sea of troubles,\n"
            "And by opposing end them? To die: to sleep;\n"
            "No more; and by a sleep to say we end\n"
            "The heart-ache and the thousand natural shocks\n"
            "That flesh is heir to, 'tis a consummation\n"
            "Devoutly to be wish'd. To die, to sleep;";

    err = dtree_addliteral(child, hamlet, REAL_STRLEN(hamlet));
    if(err) goto exit;

    /* Split our tree into two single-nodes */
    err = dtree_split_trees(root, child);
    if(err) goto exit;

    /* Re-merge because they miss each other */
    err = dtree_merge_trees(root, child);
    if(err) goto exit;

    /* Cleanup */
    exit:
    dtree_free(root);
    return err;
}

dt_err search_for_payload()
{
    dt_err err;

    dtree *root, *a, *b, *found;
    err = dtree_malloc(&root);
    if(err) goto exit;

    const char *string = "This is some data!";
    err = dtree_addrecursive(root, &a);
    if(err) goto exit;

    err = dtree_addliteral(a, string, REAL_STRLEN(string));
    if(err) goto exit;

    err = dtree_addrecursive(root, &b);
    if(err) goto exit;

    err = dtree_addnumeral(b, 1337);
    if(err) goto exit;

    /* Try to find our data again */

    err = dtree_search_payload(root, &found, (void*) string, LITERAL);
    if(err) goto exit;

    err = dtree_search_payload(root, &found, (void*) 1337, NUMERAL);
    if(err) goto exit;

    exit:
    dtree_free(root);
    return err;
}