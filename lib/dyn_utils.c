#include <dtree/dyn_tree.h>
#include <stdio.h>
#include <stdlib.h>


/*** Forward declared functions ***/

#define TRUE    1
#define FALSE   0

static int json_len = 0;

int human(short mm) {
    if(mm == DYNTREE_JSON_HUMAN) return 1;
    return 0;
}

int parse_key_value(dtree *key, char *buffer, short mode)
{
    if(key->type != LITERAL) 5;

    size_t key_len = key->used;
    int base = 3;

    /* Make an array that will survive function switch */
    char lit[key_len + base + 1];

    strcpy(buffer, "\"");
    strcat(buffer, key->payload.literal);
    strcat(buffer, "\":");
    strcat(buffer, "\0");
    return 0;
}

const char *parse_value_list(dtree *value, char *buffer, char *global, short mode, int depth, int last)
{
    if(value == NULL) return "[ERROR]";

    /* The new offset we need (in \t) */

    int no_len = depth + 1;
    char new_offset[no_len];
    int i;
    for(i = 0; i < depth + 1; i++)
    strcat(new_offset, "\t");

    int base;
//    if(human(mode)) base = 4 + no_len; // "<key>"
    base = 2;
    if(!last) base++;

    switch(value->type) {
        case LITERAL:
        {
            size_t key_len = value->used;

            strcpy(buffer, "\"");
            strcat(buffer, value->payload.literal);
            strcat(buffer, "\"");
            strcat(buffer, "\0");

            if(last == 0) strcat(buffer, ",");
            break;
        }

        case NUMERAL:
        {
            char str[15];
            sprintf(str, "%ld", value->payload.numeral);
            int val_len = (int) strlen((char*) str);

            strcat(buffer, str);
            if(last == 0) strcat(buffer, ",");
            strcat(buffer, "\0");

            break;
        }

        case RECURSIVE:
        {
            if(value->used > 0) {

                dt_uni_t test = value->payload.recursive[0]->type;

                if(test == LITERAL || test == NUMERAL) {
                    fflush(stdout);

                    int j;
                    for(j = 0; j < value->used; j++) {
                        dtree *child = value->payload.recursive[j];

                        char vall[1024];
                        parse_value_list(child, vall, global, mode, depth + 1, (i == value->used - 1) ? TRUE : FALSE);
                        fflush(stdout);
                    }

                    fflush(stdout);

                } else if(test == PAIR) {
                    fflush(stdout);
//                    *global = realloc(*global, sizeof(char) * strlen(*global) + 1);
                    append(global, "{");

                    int j;
                    for(j = 0; j < value->used; j++) {
                        dtree *child = value->payload.recursive[j];

                        if(child->type == PAIR) {
                            dtree *key = child->payload.recursive[0];
                            dtree *val = child->payload.recursive[1];

                            char kkey[1024];

                            parse_key_value(key, kkey, mode);
                            fflush(stdout);
//                            *global = realloc(*global, sizeof(char) * strlen(*global) + strlen(kkey));
                            append(global, kkey);

                            char vval[1024];
                            parse_value_list(val, vval, global, mode, 1, (j == child->used - 1) ? TRUE : FALSE);
                            fflush(stdout);

//                            *global = realloc(*global, sizeof(char) * strlen(*global) + strlen(vval));
                            append(global, vval);
                        }
                    }

                    fflush(stdout);
//                    *global = realloc(*global, sizeof(char) * strlen(*global) + 1);
                    append(global, "}");
                }

            } else {
                fflush(stdout);
            }
        }

        default: INVALID_PAYLOAD;
    }

    return "";
}

void append(char *buffer, char *message)
{
    int msg_len = (int) strlen(message);
    sprintf(buffer + json_len, message);
    json_len += msg_len;
}

/******/


const char *rdb_error_getmsg(dt_err *e)
{

}

dt_err dtree_encode_set(dtree *data, short setting)
{
    if(data == NULL) return INVALID_PARAMS;

    /* Check if setting is valid */
    switch(setting) {
        case DYNTREE_ENCODE_NONE:
        case DYNTREE_JSON_MINIFIED:
        case DYNTREE_JSON_HUMAN:
            break;

        default: return INVALID_PARAMS;
    }

    data->encset = setting;
    return SUCCESS;
}


dt_err dtree_encode_json(dtree *data, char *json_data)
{
    if(data == NULL) return INVALID_PARAMS;

    json_len = 0;

    /* Check if setting is valid */
    switch(data->encset) {
        case DYNTREE_JSON_MINIFIED:
        case DYNTREE_JSON_HUMAN:
            break;

        default: return INVALID_PARAMS;
    }

    /* Assume mode for all children */
    short mode = data->encset;

    char *open = "{";
    char *close = "}";

    if(data->type == RECURSIVE) {

        fflush(stdout);
        append(json_data, open);

        /* Iterate through all it's children */
        int i;
        for(i = 0; i < data->used; i++) {
            dtree *child = data->payload.recursive[i];

            if(child->type == PAIR) {
                dtree *key = child->payload.recursive[0];
                dtree *val = child->payload.recursive[1];

                char kkey[1024];
                parse_key_value(key, kkey, mode);;
                fflush(stdout);
                append(json_data, kkey);
                memset(kkey, 0, 1024);

                char vval[1024];
                parse_value_list(val, vval, json_data, mode, 1, (i == data->used - 1) ? TRUE : FALSE);
                fflush(stdout);

                append(json_data, vval);
                memset(vval, 0, 1024);

            } else if(child->type == RECURSIVE) {
                dt_err err = dtree_encode_json(child, json_data);
                if(err) return err;
            }
        }

    } else {
        return INVALID_PAYLOAD;
    }
    fflush(stdout);

    /* Add closing } and null terminator to finish */
    append(json_data, close);
    append(json_data, "\0");

    return SUCCESS;
}


dt_err dtree_decode_json(dtree *(*data), const char *json_data)
{

}


/**************** PRIVATE UTILITY FUNCTIONS ******************/