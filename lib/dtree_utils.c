#include <dtree/dtree.h>
#include <stdio.h>
#include <stdlib.h>

void pk_string_trim(char *src, char *dst);

dt_err dtree_decode_json(dtree *(*data), const char *json_data)
{
    enum parse_state {
        VALUE, HASH, LIST, WAITING
    };

#define BUF_SIZE 256

    /* Save some space for a token */
    const char *delims = ",:";

    char *parse;
    char curr_key[BUF_SIZE];
    char curr_val[BUF_SIZE];
    dtree *root, *curr_root;
    enum parse_state state = WAITING;

    /* Prepare environment for parsing */
    char json_buf[REAL_STRLEN(json_data)];
    strcpy(json_buf, json_data);

    /* Setup root dtree node */
    dtree_malloc(&root);
    curr_root = root;

    /* Read in the first token */
    parse = strtok(json_buf, delims);

    while(parse != NULL) {

        char tok[strlen(parse) + 1];
        memset(tok, 0, strlen(parse) + 1);

        pk_string_trim(parse, tok);

        /* Open a new hash context */
        if(tok[0] == '{') {

            dtree *new_root;
            dtree_addlist(curr_root, &new_root);

            curr_root = new_root;

            printf("Creating new hash context...\n");
            state = HASH;
        }

        /* Open a new list context - finishing a PAIR - Back to waiting */
        if(tok[0] == '[') {
            printf("Creating new hash context...\n");
            state = LIST;
        }

        /* If we're in a hash & waiting for a key */
        if(state == HASH) {
            if(tok[0] == '{')   strcpy(curr_key, tok + 1);
            else                strcpy(curr_key, tok);

            printf("Current Key: %s\n", curr_key);
            state = VALUE;
            goto END;
        }

        /* We already had a key - finishing up the pair */
        if(state == VALUE) {
            strcpy(curr_val, tok);
            printf("Current Val: %s\n", curr_val);

            /* Copy pair into dtree structure */
            dtree *parent, *key, *val;
            dtree_addlist(curr_root, &parent);

            /* Make the "parent" node into the pair parent */
            dtree_addpair(parent, &key, &val);
            dtree_addliteral(key, curr_key);
            dtree_addliteral(val, curr_val);

            /* Add the parent */

            /* Blank current pair data */
            memset(curr_key, 0, BUF_SIZE);
            memset(curr_val, 0, BUF_SIZE);

            state = HASH;
            goto END;
        }

        if(state == LIST) {
            dtree *child;
            dtree_addlist(curr_root, &child);
            dtree_addliteral(child, tok);

            size_t chs = strlen(tok);
            dtree *parent;

            dtree_parent(root, curr_root, &parent);
            if(tok[chs] == ']') {
                curr_root = parent;
                state = HASH;
            }
        }

        printf("      Recognised token: %s\n", tok);
    END:
        parse = strtok(NULL, delims);
    }

    return SUCCESS;
}


/************************************************************/

void pk_string_trim(char *src, char *dst)
{
    int s, d=0;
    for (s=0; src[s] != 0; s++)
        if (src[s] != ' ') {
            dst[d] = src[s];
            d++;
        }
    dst[d] = 0;
}