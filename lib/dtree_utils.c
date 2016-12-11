#include <dtree/dtree.h>
#include <stdio.h>
#include <stdlib.h>


/*** Forward declared functions ***/

#define TRUE    1
#define FALSE   0

static int json_len = 0;

/************* Forward Function Declarations *************/

// Functions required by encoder
int human(short );

void append(char *, char *);

int parse_key_value(dtree *, char *);

const char *parse_value_list(dtree *, char *, char *, int );

// Functions required by decoder

void append_char(char *, int *, char );

long to_long(char *);

/*********************************************************/

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

    /* Check if setting is valid */
    //    switch(data->encset) {
    //        case DYNTREE_JSON_MINIFIED:
    //        case DYNTREE_JSON_HUMAN:
    //            break;
    //
    //        default: return INVALID_PARAMS;
    //    }

    /* Assume mode for all children */
    json_len = 0;

    char *open = "{";
    char *close = "}";

    if(data->type == LIST) {

        fflush(stdout);
        append(json_data, open);

        /* Iterate through all it's children */
        int i;
        for(i = 0; i < data->used; i++) {
            dtree *child = data->payload.list[i];

            if(child->type == PAIR) {
                dtree *key = child->payload.list[0];
                dtree *val = child->payload.list[1];

                char kkey[1024];
                parse_key_value(key, kkey);
                fflush(stdout);
                append(json_data, kkey);
                memset(kkey, 0, 1024);

                char vval[1024];
                parse_value_list(val, vval, json_data, (i == data->used - 1) ? TRUE : FALSE);
                fflush(stdout);

                append(json_data, vval);
                memset(vval, 0, 1024);

            } else if(child->type == LIST) {
                dt_err err = dtree_encode_json(child, json_data);
                if(err) return err;
            }
        }

    } else {
        return INVALID_PAYLOAD;
    }

    /* Add closing } and null terminator to finish */
    append(json_data, close);
    append(json_data, "\0");

    return SUCCESS;
}


dt_err dtree_decode_json(dtree *(*data), const char *jd)
{
    /* Always create an empty root node */

    int ctr = -1;
    dtree *parents[32]; // Only support 32 deep for now
    memset(parents, 0, sizeof(dtree*) * 32);

#define IN_STRING   9
#define NEUTRAL     10

    /** Parse stack */
    int in_str = 0;
    char curr_key[512]; int key_inx = 0;
    char curr_str[512]; int str_inx = 0;
    char curr_num[512]; int num_inx = 0;

    memset(curr_key, 0, 512);
    memset(curr_str, 0, 512);
    memset(curr_num, 0, 512);

    /* Get the first character of our json string */
    int jd_len = (int) REAL_STRLEN(jd);
    int pos = 0;
    char curr;

    for (; pos < jd_len && jd[pos] != '\0'; pos++) {
        curr = jd[pos];

        switch(curr) {
            case '{':
            {
                dtree *new_root;
                dtree_malloc(&new_root);

                if(ctr < 0) {
                    parents[ctr = 0] = new_root;
                } else {
                    dtree_addlist(parents[ctr], &new_root);
                    parents[++ctr] = new_root;
                }

                break;
            }
            case '[':
            {
                if(in_str) break; // Ignore if we're in a string

                dtree *new_root;
                dtree_addlist(parents[ctr], &new_root);
                parents[++ctr] = new_root;
                break;
            }

            case '}':
            case ']':
            {
                if(in_str) {
                } else {
                    if(curr_key[0] != '\0') {

                        dtree *key, *val;
                        dtree *rec_entry;

                        dtree_addlist(parents[ctr], &rec_entry);
                        dtree_addpair(rec_entry, &key, &val);
                        dtree_addliteral(key, curr_key);
                        dtree_addliteral(val, curr_str);

                        /* Clear the pointer reference */
                        rec_entry = key = val = NULL;

                        memset(curr_key, 0, (size_t) key_inx);
                        memset(curr_str, 0, (size_t) str_inx);
                        key_inx = 0;
                        str_inx = 0;
                    }

                    if(ctr > 0) parents[ctr--] = NULL; // Remove current parent again
                }
                break;
            }

            case '"':
            {
                in_str = (in_str) ? FALSE : TRUE;
                break;
            }

            case ',':
            {
                dtree *key, *val;
                dtree *rec_entry;

                /* Add a new pair as a list item */
                dtree_addlist(parents[ctr], &rec_entry);
                dtree_addpair(rec_entry, &key, &val);
                dtree_addliteral(key, curr_key);

                /* Either make it a literal or number node */
                if(num_inx > 0)
                    dtree_addnumeral(val, to_long(curr_num));
                else
                    dtree_addliteral(val, curr_str);

                /* Clear the pointer reference */
                rec_entry = key = val = NULL;

                /* Reset the key/ value status */
                memset(curr_key, 0, (size_t) key_inx);
                memset(curr_str, 0, (size_t) str_inx);
                memset(curr_num, 0, (size_t) num_inx);
                key_inx = 0;
                str_inx = 0;
                num_inx = 0;
                break;
            }

            case ':':
            {
                if(in_str) break; // Ignore if we're in a string

                // End a key
                strcpy(curr_key, curr_str);
                memset(curr_str, 0, (size_t) str_inx);
                key_inx = str_inx;
                str_inx = 0;
                break;
            }

            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
            {
                if(in_str) {
                    append_char(curr_str, &str_inx, curr);
                } else {
                    append_char(curr_num, &num_inx, curr);
                }
                break;
            }

            default:
            {
                if(in_str) append_char(curr_str, &str_inx, curr);
                break;
            }
        }
    }

    /* Allocate our first node */
    *data = parents[0];
    dtree_print(*data);

    return SUCCESS;
}


/**************** ENCODER UTILITY FUNCTIONS ******************/

int parse_key_value(dtree *key, char *buffer)
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

const char *parse_value_list(dtree *value, char *buffer, char *global, int last)
{
    if(value == NULL) return "[ERROR]";

    /* The new offset we need (in \t) */
    int i;

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

        case NUMERIC:
        {
            char str[15];
            sprintf(str, "%ld", value->payload.numeral);

            strcat(buffer, str);
            if(last == 0) strcat(buffer, ",");
            strcat(buffer, "\0");

            break;
        }

        case LIST:
        {
            if(value->used > 0) {

                dt_uni_t test = value->payload.list[0]->type;

                if(test == LITERAL || test == NUMERIC) {
                    fflush(stdout);

                    int j;
                    for(j = 0; j < value->used; j++) {
                        dtree *child = value->payload.list[j];

                        char vall[1024];
                        parse_value_list(child, vall, global, (j == value->used - 1) ? TRUE : FALSE);
                        fflush(stdout);
                    }

                    fflush(stdout);

                } else if(test == PAIR) {
                    fflush(stdout);
                    append(global, "{");

                    int j;
                    for(j = 0; j < value->used; j++) {
                        dtree *child = value->payload.list[j];

                        if(child->type == PAIR) {
                            dtree *key = child->payload.list[0];
                            dtree *val = child->payload.list[1];

                            char kkey[1024];

                            parse_key_value(key, kkey);
                            fflush(stdout);
                            append(global, kkey);

                            char vval[1024];
                            parse_value_list(val, vval, global, (j == child->used - 1) ? TRUE : FALSE);
                            fflush(stdout);

                            append(global, vval);
                        }
                    }

                    fflush(stdout);
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

/**************** DECODER UTILITY FUNCTIONS ******************/

void append_char(char *buffer, int *ctr, char c)
{
    sprintf(buffer + (*ctr), "%c", c);
    (*ctr)++;
}

long to_long(char *buffer)
{
    return atol(buffer);
}
