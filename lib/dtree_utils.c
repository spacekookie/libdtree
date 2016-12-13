#include <dtree/dtree.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <jansson.h>

#include "jsmn.h"


#define DTREE_TOK_BOOLEAN   (1 << 1)
#define DTREE_TOK_NUMERICAL (1 << 2)
#define DTREE_TOK_LITERAL   (1 << 3)

#define TOK_PAIR_KEYED      (1 << 11)


char *tok_to_str(jsmntype_t *tok)
{
    switch(*tok) {
        case JSMN_UNDEFINED: return "UNDEFINED";
        case JSMN_OBJECT: return "OBJECT";
        case JSMN_ARRAY: return "ARRAY";
        case JSMN_STRING: return "STRING";
        case JSMN_PRIMITIVE: return "PRIMITIVE";
        default: return "UNKNOWN";
    }
}


int digest_payload(const char *token)
{
    char* end;
    size_t len = strlen(token);

    if(len == strlen("true") || len == strlen("false"))
        if(strcmp(token, "true") == 0 || strcmp(token, "false") == 0)
            return DTREE_TOK_BOOLEAN;

    /* It could still be a number! */
    strtol(token, &end, 10);
    if (!*end) return DTREE_TOK_NUMERICAL;

    return DTREE_TOK_LITERAL;
}


dt_err dtree_decode_json(dtree *(*data), const char *json_data, size_t len)
{
    jsmn_parser parse;
    jsmn_init(&parse);

    // FIXME: Variable amount of tokens?
    unsigned int no_tokens = 1024 * 32;
    jsmntok_t *tokens = malloc(sizeof(jsmntok_t) * no_tokens);
    memset(tokens, 0, sizeof(jsmntok_t) * no_tokens);

    int ret = jsmn_parse(&parse, json_data, strlen(json_data), tokens, no_tokens);

    unsigned int idx = 0;
    jsmntok_t tok;

    /** Prepare dtree nodes */
    dtree *root, *curr;
    dtree_malloc(&root);
    curr = root;

    struct bounds {
        int low, high;
    };

    struct pair {
        short   state;
        char    key[1024];
        union   value {
            char            string[1024];
            unsigned long   num;
        } value;
    };

    /* Save some space to store token bounds */
    struct bounds *bounds = malloc(sizeof(struct bounds) * len);
    memset(bounds, 0, sizeof(struct bounds) * len);

    /* Have a structure to record array types in the tree */
    bool *is_array = malloc(sizeof(bool) * len);
    memset(bounds, 0, sizeof(bool) * len);

    /* Set the currently focused node */
    int focused = -1;

    struct pair c_pair;
    memset(&c_pair, 0, sizeof(struct pair));

    while(tok = tokens[idx++], tok.type != NULL) {

        size_t tok_len = (size_t) tok.end - tok.start;
        char token[tok_len + 1];
        memset(token, 0, tok_len + 1);
        memcpy(token, json_data + tok.start, tok_len);

        /** Check if we need to move the boundry scope (again) */
        if(focused > 0 && tok.end >= bounds[focused].high) {
            focused--;

            /* Because of how our root node is a VALUE node, we need the parents parent */
            dtree *parent, *pair_parent;
            dtree_parent(root, curr, &parent);
            dtree_parent(root, parent, &pair_parent);

            /* Assign the new root node - old scope restored */
            curr = pair_parent;
        }

        switch(tok.type) {

             /**
             * When we encounter a new json object, shift our "focus" over by one so we can
             * record in what range this object is going to accumilate tokens.
             *
             * We then create a new child node under the current root node and switch the
             * curr root pointer over to that child.
             *
             * When we reach the end of the token scope, we need to re-reference the parent as
             * current root and switch over our boundry scope as well. This is done before the
             * parsing switch statement.
             */
            case JSMN_ARRAY:
            case JSMN_OBJECT:
            {
                focused++;
                bounds[focused].low = tok.start;
                bounds[focused].high = tok.end;

                /* This is not elegant at all! */
                if(tok.type == JSMN_ARRAY) is_array[focused] = true;



                /**
                 * Most of the time, we will create a new object under the key of
                 * a pair. This is the case, when the c_pair state buffer has been
                 * set to KEYED. In this case we allocate a new pair node for key
                 * and value and set that value to the new root.
                 */
                if(c_pair.state == TOK_PAIR_KEYED) {

                    /* Create pair nodes & new_root which becomes curr */
                    dtree *pair, *key, *val;
                    dtree_addlist(curr, &pair);
                    dtree_addpair(pair, &key, &val);

                    /* Assign key and new_root as a value of the pair */
                    dtree_addliteral(key, c_pair.key);

                    /* Move curr root pointer */
                    curr = val;

                    /* Blank c_pair data for next tokens */
                    memset(&c_pair, 0, sizeof(struct pair));
                }

                /* Skip to next token */
                continue;
            }

            case JSMN_PRIMITIVE:
            case JSMN_STRING:
            {

                /**
                 * First check if we are currently dealing with an array. If we are
                 * the way that we create nodes changes. Every token is immediately added
                 * to the currently focused list node
                 */
                if(is_array[focused]) {

                    dtree *val;
                    dtree_addlist(curr, &val);

                    /* Parse payload and asign to value node */
                    switch(digest_payload(token)) {
                        case DTREE_TOK_LITERAL:
                            dtree_addliteral(val, token);
                            break;

                        case DTREE_TOK_NUMERICAL:
                            dtree_addnumeral(val, atol(token));
                            break;

                        case DTREE_TOK_BOOLEAN:
                            dtree_addboolean(val, (strcpy(token, "true") == 0) ? true : false);
                            break;

                        default: continue;
                    }

                    /* Blank c_pair data for next tokens */
                    memset(&c_pair, 0, sizeof(struct pair));

                } else {

                    /**
                     * Here we need to check if we are adding a string as a key
                     * or as a value. This is simply done by checking for the existance
                     * of a key in the c_pair (current pair) variable.
                     *
                     * We know the token positions so we can manualy copy from the json stream
                     */
                    if(c_pair.state == 0) {
                        memcpy(c_pair.key, json_data + tok.start, (size_t) tok.end - tok.start);
                        c_pair.state = TOK_PAIR_KEYED;

                    } else if(c_pair.state == TOK_PAIR_KEYED){

                        /** Create a PAIR node under current root */
                        dtree *pair, *key, *val;
                        dtree_addlist(curr, &pair);
                        dtree_addpair(pair, &key, &val);

                        /* Key is always literal */
                        dtree_addliteral(key, c_pair.key);

                        /* Parse payload and asign to value node */
                        switch(digest_payload(token)) {
                            case DTREE_TOK_LITERAL:
                                dtree_addliteral(val, token);
                                break;

                            case DTREE_TOK_NUMERICAL:
                                dtree_addnumeral(val, atol(token));
                                break;

                            case DTREE_TOK_BOOLEAN:
                                dtree_addboolean(val, (strcpy(token, "true") == 0) ? true : false);
                                break;

                            default: continue;
                        }

                        /* Blank c_pair data for next tokens */
                        memset(&c_pair, 0, sizeof(struct pair));
                    }

                }

                /* Skip to next token */
                continue;
            }



            default:
                continue;
        }
    }

    /* Switch over data pointer and return */
    (*data) = root;
    return SUCCESS;
}