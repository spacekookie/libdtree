
#include <stdio.h>
#include <dtree/dyn_tree.h>

/**
 * A small test that creates a tree, splits the nodes
 * and then merges them again.
 */
dt_err split_and_merge();

dt_err search_for_payload();

dt_err json_encode(char *json);

dt_err test_shortcut_functions();

#define TEST(function) \
    printf("Running '%s'...", #function); \
    fflush(stdout); \
    err = function; \
    printf(" %s\n", (err == 0) ? "OK!" : "FAILED!"); \
    if(err) goto end;

int main(void)
{
    dt_err err;
    printf("=== libdyntree test suite ===\n");

    /* Search inside trees */
    TEST(search_for_payload())

    /* Split and merge trees */
    TEST(split_and_merge())

    /* Test shortcut functions */
    TEST(test_shortcut_functions())

    /* Try to encode a structure into json */
    char json[512]; // Provide a buffer that is big enough
    TEST(json_encode(json))

    printf("\n\nJson string: %s\n", json);

    dtree *data;
    dtree_decode_json(&data, json);

    end:
    exit:
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

dt_err json_encode(char *json) {
    dt_err err;

    dtree *root, *a, *b, *c, *found;
    err = dtree_malloc(&root);
    if (err) goto exit;

    dtree *key, *val;
    err = dtree_addrecursive(root, &a);
    if (err) goto exit;
    err = dtree_addrecursive(root, &b);
    if (err) goto exit;
    err = dtree_addrecursive(root, &c);
    if (err) goto exit;

    err = dtree_addpair(a, &key, &val);
    if (err) goto exit;
    err = dtree_addliteral(key, "Server Address", REAL_STRLEN("Server Address"));
    if (err) goto exit;
    err = dtree_addliteral(val, "https://github.com", REAL_STRLEN("https://github.com"));
    if (err) goto exit;

    key = val = NULL;

    err = dtree_addpair(b, &key, &val);
    if (err) goto exit;
    err = dtree_addliteral(key, "Server Port", REAL_STRLEN("Server Port"));
    if (err) goto exit;
    err = dtree_addnumeral(val, 8080);
    if (err) goto exit;

    key = val = NULL;

    err = dtree_addpair(c, &key, &val);
    if (err) goto exit;
    err = dtree_addliteral(key, "Users", REAL_STRLEN("Users"));
    if (err) goto exit;

    dtree *sbrec, *sbrec2;
    err = dtree_addrecursive(val, &sbrec);
    if (err) goto exit;
    err = dtree_addrecursive(val, &sbrec2);
    if (err) goto exit;

    dtree *subkey, *subval;
    err = dtree_addpair(sbrec, &subkey, &subval);
    if (err) goto exit;
    err = dtree_addliteral(subkey, "spacekookie", REAL_STRLEN("spacekookie"));
    if (err) goto exit;
    err = dtree_addliteral(subval, "Admin", REAL_STRLEN("Admin"));
    if (err) goto exit;

    key = val = NULL;

    dtree *subkey2, *subval2;
    err = dtree_addpair(sbrec2, &subkey2, &subval2);
    if (err) goto exit;
    err = dtree_addliteral(subkey2, "jane", REAL_STRLEN("jane"));
    if (err) goto exit;
    err = dtree_addliteral(subval2, "normal", REAL_STRLEN("normal"));
    if (err) goto exit;

    err = dtree_encode_set(root, DYNTREE_JSON_MINIFIED);
    if (err) goto exit;
    err = dtree_encode_json(root, json);
    if (err) goto exit;

    exit:
    dtree_free(root);
    return err;
}

dt_err test_shortcut_functions()
{
    dtree *key, *val;
    dtree *root = dtree_allocpair_new(&key, &val);

    dtree_addliteral(key, "Address", REAL_STRLEN("Address"));
//    dtree_addliteral(val, "Berlin 10555", REAL_STRLEN("Berlin 10555"));

    dtree *list_root;
    dtree **list = dtree_alloc_reclist(&list_root, 4); // Allocate 4 nodes

    int i;
    for(i = 0; i < 4; i++){
        char message[32];
        sprintf(message, "Message no.%d", i);
        dtree_addliteral(list[i], message, REAL_STRLEN(message));
    }

    dtree_merge_trees(val, list_root);

    dtree_free(root);
    return SUCCESS; // We think
}