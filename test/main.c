
#include <stdio.h>

#include <dtree/dtree.h>
#include <dtree/eztree.h>
#include <time.h>
#include <sys/time.h>

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

int main(int argn, char **argv)
{
    dt_err err = SUCCESS;
    printf("=== libdyntree test suite ===\n");

//    TEST(split_and_merge())
//
//    TEST(search_for_payload())
//
//    char json[1024];
//    TEST(json_encode(json))
//
//    dtree *recover;
//    dtree_decode_json(&recover, json);
//    dtree_free(recover);

    struct timeval t1, t2;
    double elapsedTime;

    // start timer
    gettimeofday(&t1, NULL);

#define PATH "/home/spacekookie/Downloads/generated.json"

    /* Open the file and seek through it for length */
    FILE *f = fopen(PATH, "r");
    fseek(f, 0, SEEK_END);
    size_t file_size = (size_t) ftell(f);
    fseek(f, 0, SEEK_SET);

    /* Create a buffer of the correct size */
    char *json = (char*) malloc(sizeof(char) * (file_size + 1));
    memset(json, 0, file_size + 1);
    fread(json, file_size, 1, f);
    fclose(f);

    dtree *recov;
    dtree_decode_json(&recov, json, file_size);

    dtree_print(recov);
    dtree_free(recov);

    gettimeofday(&t2, NULL);

    // compute and print the elapsed time in millisec
    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms

    printf("Program took %fms to run\n", elapsedTime);


end:
    printf("==== done ====\n");
    return err;
}


/*************** TEST IMPLEMENTATIONS ****************/

//dt_err split_and_merge()
//{
//    dt_err err;
//
//    /* Allocate a node named root */
//    dtree *root;
//    err = dtree_malloc(&root);
//    if(err) goto exit;
//
//    /* Add child as a recursive node to root */
//    dtree *child;
//    err = dtree_addlist(root, &child);
//    if(err) goto exit;
//
//    /* Make child a literal node containing the works of shakespeare */
//    const char *hamlet = "To be, or not to be: that is the question:\n"
//            "Whether 'tis nobler in the mind to suffer\n"
//            "The slings and arrows of outrageous fortune,\n"
//            "Or to take arms against a sea of troubles,\n"
//            "And by opposing end them? To die: to sleep;\n"
//            "No more; and by a sleep to say we end\n"
//            "The heart-ache and the thousand natural shocks\n"
//            "That flesh is heir to, 'tis a consummation\n"
//            "Devoutly to be wish'd. To die, to sleep;";
//
//    err = dtree_addliteral(child, hamlet);
//    if(err) goto exit;
//
//    /* Split our tree into two single-nodes */
//    err = dtree_split_trees(root, child);
//    if(err) goto exit;
//
//    /* Re-merge because they miss each other */
//    err = dtree_merge_trees(root, child);
//    if(err) goto exit;
//
//    /* Cleanup */
//    exit:
//    dtree_free(root);
//    return err;
//}
//
//dt_err search_for_payload()
//{
//    dt_err err;
//
//    dtree *root, *a, *b, *found;
//    err = dtree_malloc(&root);
//    if(err) goto exit;
//
//    const char *string = "This is some data!";
//    err = dtree_addlist(root, &a);
//    if(err) goto exit;
//
//    err = dtree_addliteral(a, string);
//    if(err) goto exit;
//
//    err = dtree_addlist(root, &b);
//    if(err) goto exit;
//
//    err = dtree_addnumeral(b, 1337);
//    if(err) goto exit;
//
//    /* Try to find our data again */
//
//    err = dtree_search_payload(root, &found, (void*) string, LITERAL);
//    if(err) goto exit;
//
//    err = dtree_search_payload(root, &found, (void*) 1337, NUMERIC);
//    if(err) goto exit;
//
//    exit:
//    dtree_free(root);
//    return err;
//}
//
//dt_err json_encode(char *json) {
//    dt_err err;
//
//    dtree *root, *a, *b, *c, *found;
//    err = dtree_malloc(&root);
//    if (err) goto exit;
//
//    dtree *key, *val;
//    err = dtree_addlist(root, &a);
//    if (err) goto exit;
//    err = dtree_addlist(root, &b);
//    if (err) goto exit;
//    err = dtree_addlist(root, &c);
//    if (err) goto exit;
//
//    err = dtree_addpair(a, &key, &val);
//    if (err) goto exit;
//    err = dtree_addliteral(key, "Server Address");
//    if (err) goto exit;
//    err = dtree_addliteral(val, "https://github.com");
//    if (err) goto exit;
//
//    key = val = NULL;
//
//    err = dtree_addpair(b, &key, &val);
//    if (err) goto exit;
//    err = dtree_addliteral(key, "Server Port");
//    if (err) goto exit;
//    err = dtree_addnumeral(val, 8080);
//    if (err) goto exit;
//
//    key = val = NULL;
//
//    err = dtree_addpair(c, &key, &val);
//    if (err) goto exit;
//    err = dtree_addliteral(key, "Users");
//    if (err) goto exit;
//
//    dtree *sbrec, *sbrec2;
//    err = dtree_addlist(val, &sbrec);
//    if (err) goto exit;
//    err = dtree_addlist(val, &sbrec2);
//    if (err) goto exit;
//
//    dtree *subkey, *subval;
//    err = dtree_addpair(sbrec, &subkey, &subval);
//    if (err) goto exit;
//    err = dtree_addliteral(subkey, "spacekookie");
//    if (err) goto exit;
//    err = dtree_addliteral(subval, "Admin");
//    if (err) goto exit;
//
//    key = val = NULL;
//
//    dtree *subkey2, *subval2;
//    err = dtree_addpair(sbrec2, &subkey2, &subval2);
//    if (err) goto exit;
//    err = dtree_addliteral(subkey2, "jane");
//    if (err) goto exit;
//    err = dtree_addliteral(subval2, "normal");
//    if (err) goto exit;
//
//    err = dtree_encode_set(root, DYNTREE_JSON_MINIFIED);
//    if (err) goto exit;
//    err = dtree_encode_json(root, json);
//    if (err) goto exit;
//
//    exit:
//    dtree_free(root);
//    return err;
//}
