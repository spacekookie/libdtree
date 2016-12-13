// Include our header file
#include <dtree/dtree.h>

// Runtime includes
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <stdbool.h>

#define RDB_REC_DEF_SIZE    2
#define RDB_REC_MULTIPLY    2

#define ORIGINAL            (short) 0
#define SHALLOW             (short) 1
#define DEEP                (short) 2

/*** Forward declared functions ***/

int list_search(dtree**, dtree *, dtree *);

void list_print(dtree *data, const char *offset);

/******/


dt_err dtree_malloc(dtree *(*data))
{
    (*data) = (dtree*) malloc(sizeof(dtree));
    if(*data == NULL) {
        printf("Creating dtree object FAILED");
        return MALLOC_FAILED;
    }

    memset(*data, 0, sizeof(dtree));

    (*data)->type = UNSET;
    return SUCCESS;
}

dt_err dtree_resettype(dtree *data)
{
    if(data->type == LITERAL) {
        if(data->payload.literal) free(data->payload.literal);

    } else if(data->type == LIST || data->type == PAIR) {

        /* Iterate over all children and clear them */
        int i;
        dt_err err;
        for(i = 0; i < data->size; i++) {
            err = dtree_free(data->payload.list[i]);
            if(err) return err;
        }
    }

    /* Set the data type to unset */
    data->type = UNSET;
    data->encset = 0;
    data->size = 0;
    data->used = 0;

    /* Forcibly clean union memory to avoid bleeding data */
    memset(&data->payload, 0, sizeof(data->payload));

    return SUCCESS;
}

dt_err dtree_addliteral(dtree *data, const char *literal)
{
    /* Make sure we are a literal or unset data object */
    if(data->type != UNSET)
        if(data->type != LITERAL) return INVALID_PAYLOAD;

    size_t length = REAL_STRLEN(literal);

    /* Get rid of previous data */
    if(data->payload.literal) free(data->payload.literal);

    /* Allocate space for the data */
    char *tmp = (char *) malloc(sizeof(char) * length);
    if(tmp == NULL) {
        printf("Allocating space for literal data FAILED");
        return MALLOC_FAILED;
    }

    /* Copy the string over and store it in the union */
    strcpy(tmp, literal);
    data->payload.literal = tmp;
    data->type = LITERAL;
    data->size = length;
    data->used = length;

    return SUCCESS;
}


dt_err dtree_addpointer(dtree *data, void *ptr)
{
    if(data->type != UNSET)
        if(data->type != POINTER) return INVALID_PAYLOAD;

    data->payload.pointer = ptr;
    data->type = POINTER;
    data->size = sizeof(ptr);
    data->used = sizeof(ptr);

    return SUCCESS;
}


dt_err dtree_addnumeral(dtree *data, long numeral)
{
    /* Make sure we are a literal or unset data object */
    if(data->type != UNSET)
        if(data->type != NUMERIC) return INVALID_PAYLOAD;

    data->payload.numeral = numeral;
    data->type = NUMERIC;
    data->size = sizeof(int);
    data->used = sizeof(int);
    return SUCCESS;
}


dt_err dtree_addboolean(dtree *data, bool b)
{
    /* Make sure we are a literal or unset data object */
    if(data->type != UNSET)
        if(data->type != BOOLEAN) return INVALID_PAYLOAD;

    data->payload.boolean = b;
    data->type = BOOLEAN;
    data->size = sizeof(bool);
    data->used = sizeof(bool);
    return SUCCESS;
}


dt_err dtree_addlist(dtree *data, dtree *(*new_data))
{
    /* Make sure we are a literal or unset data object */
    if(data->type != UNSET)
        if(data->type != LIST) return INVALID_PAYLOAD;

    dt_err err;

    /* This means elements already exist */
    if(data->size > 0) {

        /* Used should never > size */
        if(data->used >= data->size) {
            data->size += RDB_REC_MULTIPLY;

            // TODO Use Realloc
            dtree **tmp = (dtree**) malloc(sizeof(dtree*) * data->size);
            memcpy(tmp, data->payload.list, sizeof(dtree*) * data->used);

            /* Free the list WITHOUT the children! */
            free(data->payload.list);
            data->payload.list = tmp;
        }

    /* This means the data object is new */
    } else {
        dtree **tmp = (dtree**) malloc(sizeof(dtree*) * RDB_REC_DEF_SIZE);
        data->payload.list = tmp;
        data->type = LIST;
        data->used = 0;
        data->size = RDB_REC_DEF_SIZE;
    }

    err = dtree_malloc(new_data);
    if(err) return err;

    /* Reference the slot, assign it, then move our ctr */
    data->payload.list[data->used++] = *new_data;

    return SUCCESS;
}


dt_err dtree_addpair(dtree *data, dtree *(*key), dtree *(*value))
{
    /* Make sure we are a literal or unset data object */
    if(data->type != UNSET) return INVALID_PAYLOAD;

    dt_err err;

    /* Malloc two nodes */
    err = dtree_malloc(key);
    if(err) goto cleanup;

    err = dtree_malloc(value);
    if(err) goto cleanup;

    /** Malloc space for PAIR */
    data->size = 2;
    dtree **tmp = (dtree**) malloc(sizeof(dtree*) * data->size);
    if(!tmp) goto cleanup;

    data->payload.list = tmp;

    { /* Assign data to new array */
        data->payload.list[data->used] = *key;
        data->used++;
        data->payload.list[data->used] = *value;
        data->used++;
    }

    /* Assign our new type and return */
    data->type = PAIR;
    return SUCCESS;

    /* Code we run when we can't allocate structs anymore */
    cleanup:
    free(*key);
    free(*value);
    free(tmp);
    return MALLOC_FAILED;
}


dt_err dtree_split_trees(dtree *data, dtree *sp)
{
    /* Make sure we are a literal or unset data object */
    if(data->type == UNSET) return INVALID_PAYLOAD;

    /* Check that sp is really a child of data */
    dtree *dp;
    int ret = list_search(&dp, data, sp);
    if(ret != 0) return DATA_NOT_RELATED;
    if(dp == NULL) return NODE_NOT_FOUND;

    /* Find the exact list reference and remove it */
    int i;
    for(i = 0; i < dp->used; i++) {
        if(dp->payload.list[i] == NULL) continue;

        /* Manually remove the entry */
        if(dp->payload.list[i] == sp) {
            dp->used--;
            dp->payload.list[i] = NULL;
        }
    }

    return SUCCESS;
}


dt_err dtree_merge_trees(dtree *data, dtree *merge)
{
    /* REALLY make sure the type is correct */
    if(data->type == UNSET) return INVALID_PARAMS;
    if(!(data->type == LIST || data->type == PAIR))
        return INVALID_PAYLOAD;

    /* This means elements already exist */
    if(data->size > 0) {

        /* Used should never > size */
        if(data->used >= data->size) {
            data->size += RDB_REC_MULTIPLY;

            dtree **tmp = (dtree**) malloc(sizeof(dtree*) * data->size);
            memcpy(tmp, data->payload.list, sizeof(dtree*) * data->used);

            /* Free the list WITHOUT the children! */
            free(data->payload.list);
            data->payload.list = tmp;
        }

        /* This means the data object is new */
    } else {
        dtree **tmp = (dtree**) malloc(sizeof(dtree*) * data->size);
        data->payload.list = tmp;
        data->type = LIST;
        data->used = 0;
        data->size = RDB_REC_DEF_SIZE;
    }

    /* Reference the slot, assign it, then move our ctr */
    data->payload.list[data->used] = merge;
    data->used++;

    return SUCCESS;
}


dt_err dtree_copy_deep(dtree *data, dtree *(*copy))
{
    if(data == NULL) return INVALID_PARAMS;
    dt_err err = SUCCESS;

    int it_type = -1;
    dt_uni_t type = data->type;

    /* Check if we're the first call */
    if((*copy) == NULL) dtree_malloc(copy);
    (*copy)->copy = DEEP;

    switch(type) {
        case LITERAL:
            dtree_addliteral(*copy, data->payload.literal);
            break;

        case NUMERIC:
            dtree_addnumeral(*copy, data->payload.numeral);
            break;

        case BOOLEAN:
            dtree_addboolean(*copy, data->payload.boolean);
            break;

        case LIST:
        {
            int i;
            int num = (int) data->used;

            for(i = 0; i < num; i++) {
                dtree *node = data->payload.list[i];

                dtree *new;
                dtree_addlist(*copy, &new);
                dtree_copy_deep(node, &new);
            }

            break;
        }

        case PAIR:
        {
            dtree *key, *val;
            dtree_addpair(*copy, &key, &val);

            dtree *orig_key = data->payload.list[0];
            dtree *orig_val = data->payload.list[1];

            dtree_copy_deep(orig_key, &key);
            dtree_copy_deep(orig_val, &val);

            break;
        }

        case POINTER:
            dtree_addpointer(*copy, data->payload.pointer);
            break;

        default:
            err = INVALID_PAYLOAD;
            break;
    }

    return err;
}


dt_err dtree_parent(dtree *root, dtree *data, dtree **parent)
{
    if(root == NULL || data == NULL) return INVALID_PARAMS;

    /* Blank the search pointer for easy error checking */
    (*parent) = NULL;

    switch(root->type) {

        /* Dead-end data stores automatically return @{NODE_NOT_FOUND} */
        case POINTER:
        case LITERAL:
        case BOOLEAN:
        case NUMERIC:
            return NODE_NOT_FOUND;

        case PAIR:
        case LIST:
            {
                int i;
                for(i = 0; i < root->used; i++) {

                    /* Check if the node we're looking at is what we're searching for */
                    if(root->payload.list[i] == data) {
                        (*parent) = root;
                        return SUCCESS;
                    }

                    dt_err err = dtree_parent(root->payload.list[i], data, parent);
                    if(err == SUCCESS) return SUCCESS;
                }
            }
            break;

        default:
            return INVALID_PAYLOAD;
    }

    return NODE_NOT_FOUND;
}


dt_err dtree_copy(dtree *data, dtree *(*copy))
{
    if(data == NULL) return INVALID_PARAMS;
    dt_err err = SUCCESS;

    /* Allocate a new node */
    err = dtree_malloc(copy);
    if(err) goto exit;

    /* Mark as shallow copy */
    (*copy)->copy = SHALLOW;

    /* Find out how to handle specific payloads */
    switch(data->type) {
        case LITERAL:
            err = dtree_addliteral(*copy, data->payload.literal);
            break;

        case NUMERIC:
            err = dtree_addnumeral(*copy, data->payload.numeral);
            break;

        case BOOLEAN:
            err = dtree_addboolean(*copy, data->payload.boolean);
            break;

        case LIST:
            (*copy)->type = LIST;
            (*copy)->payload.list = (dtree**) malloc(sizeof(dtree*) * data->size);
            memcpy((*copy)->payload.list, data->payload.list, sizeof(dtree*) * data->used);
            break;

        case PAIR:
            (*copy)->type = PAIR;
            (*copy)->payload.list = (dtree**) malloc(sizeof(dtree*) * data->size);
            memcpy((*copy)->payload.list, data->payload.list, sizeof(dtree*) * data->used);
            break;

        case POINTER:
            (*copy)->type = POINTER;
            memcpy((*copy)->payload.pointer, data->payload.pointer, sizeof(void*));
            break;

        default:
            return INVALID_PAYLOAD;
    }

    exit:
    return err;
}


dt_err dtree_search_payload(dtree *data, dtree *(*found), void *payload, dt_uni_t type)
{
    if(data == NULL) return INVALID_PARAMS;

    /* Make sure our pointer is clean */
    *found = NULL;

    if(data->type == LIST|| data->type == PAIR) {

        int i;
        for(i = 0; i < data->used; i++) {
            dt_err err = dtree_search_payload(data->payload.list[i], found, payload, type);
            if(err == SUCCESS) return SUCCESS;
        }

    } else {

        /* Check the type aligns */
        if(data->type != type) return NODE_NOT_FOUND;

        switch(type) {
            case LITERAL:
                if(strcmp(data->payload.literal, (char*) payload) == 0)
                    *found = data;
                break;

            case NUMERIC:
                if(data->payload.numeral == (long) payload)
                    *found = data;
                break;

            case BOOLEAN:
                if(data->payload.boolean == (bool) payload)
                    *found = data;
                break;

            case POINTER:
                if(data->payload.pointer == payload)
                    *found = data;
                break;

            default: return NODE_NOT_FOUND;
        }

    }

    return (*found == NULL) ? NODE_NOT_FOUND : SUCCESS;
}

// FIXME: This is horrible. Do via context?
static int reached = 0;
dt_err dtree_search_keypayload(dtree *data, dtree *(*found), void *payload, dt_uni_t type, int depth)
{
    if(data == NULL) return INVALID_PARAMS;
    if(reached++ >= depth) return QUERY_TOO_DEEP;

    /* Make sure our pointer is clean */
    *found = NULL;

    /* We can only search LISTed values or PAIRs */
    if(data->type == PAIR) {
        dtree *key = data->payload.list[0];

        dt_uni_t tt;
        int hit = -1;

        if(strcmp(key->payload.literal, (char*) payload) == 0) {
            tt = LITERAL;
            hit = 0;
        }

        if(key->payload.numeral == (long) payload) {
            tt = NUMERIC;
            hit = 0;
        }

        if(key->payload.boolean == (bool) payload) {
            tt = BOOLEAN;
            hit = 0;
        }

        if(hit == 0) *found = data->payload.list[1];

    } else if(data->type == LIST) {

        int i;
        for(i = 0; i < data->used; i++) {
            dtree_search_keypayload(data->payload.list[i], found, payload, type, depth);
        }

    } else {


    }

    if(data->type == LIST|| data->type == PAIR) {

        int i;
        for(i = 0; i < data->used; i++) {
            dt_err err = dtree_search_payload(data->payload.list[i], found, payload, type);
            if(err == SUCCESS) return SUCCESS;
        }

    } else {

        /* Check the type aligns */
        if(data->type != type) return NODE_NOT_FOUND;

        switch(type) {
            case LITERAL:
                if(strcmp(data->payload.literal, (char*) payload) == 0)
                    *found = data;
                break;

            case NUMERIC:
                if(data->payload.numeral == (long) payload)
                    *found = data;
                break;

            case BOOLEAN:
                if(data->payload.boolean == (bool) payload)
                    *found = data;

            case POINTER:
                if(data->payload.pointer == payload)
                    *found = data;
                break;

            default: return NODE_NOT_FOUND;
        }

    }

    return (*found == NULL) ? NODE_NOT_FOUND : SUCCESS;
}


void list_print(dtree *data, const char *offset)
{
    dt_uni_t type = data->type;

    switch(type) {
        case UNSET:
            printf("[NULL]\n");
            break;

        case LITERAL:
            printf("%s['%s']\n", offset, data->payload.literal);
            break;

        case NUMERIC:
            printf("%s[%lu]\n", offset, data->payload.numeral);
            break;

        case BOOLEAN:
            printf("%s['%s']\n", offset, (data->payload.boolean) ? "TRUE" : "FALSE");
            break;

        case PAIR:
        {
            dt_uni_t k_type = data->payload.list[0]->type;
            dt_uni_t v_type = data->payload.list[1]->type;

            if(k_type == LITERAL) printf("%s['%s']", offset, data->payload.list[0]->payload.literal);
            if(k_type == NUMERIC) printf("%s[%lu]", offset, data->payload.list[0]->payload.numeral);
            if(k_type == BOOLEAN) printf("%s[%s]", offset, (data->payload.list[0]->payload.boolean) ? "TRUE" : "FALSE");

            char new_offset[REAL_STRLEN(offset) + 2];
            strcpy(new_offset, offset);
            strcat(new_offset, "  ");

            if(k_type == LIST || k_type == PAIR) list_print(data->payload.list[0], new_offset);

            /* Print the value now */
            if(v_type == LITERAL) printf(" => ['%s']\n", data->payload.list[1]->payload.literal);
            if(v_type== NUMERIC) printf(" => [%lu]\n", data->payload.list[1]->payload.numeral);
            if(v_type == BOOLEAN) printf(" => [%s]\n", (data->payload.list[1]->payload.boolean) ? "TRUE" : "FALSE");

            if(v_type == LIST || k_type == PAIR) list_print(data->payload.list[1], new_offset);

            break;
        }

        case LIST:
        {
            int i;
            printf("%s[LIST]\n", offset);
            for(i = 0; i < data->used; i++) {
                dt_uni_t t = data->payload.list[i]->type;

                /* Calculate the new offset */
                char new_offset[REAL_STRLEN(offset) + 2];
                strcpy(new_offset, offset);
                strcat(new_offset, "  ");

                switch(t) {
                    case LITERAL:
                    case BOOLEAN:
                    case NUMERIC:
                        list_print(data->payload.list[i], new_offset);
                        continue;

                    case LIST:
                        list_print(data->payload.list[i], new_offset);
                        continue;

                    case PAIR:
                        printf("%s[PAIR] <==> ", new_offset);
                        list_print(data->payload.list[i], new_offset);
                        continue;

                    default:
                        break;
                }
            }
            break;
        }

        default:
            break;

    }
}


void dtree_print(dtree *data)
{
    list_print(data, "");
}

dt_err dtree_get(dtree *data, void *(*val))
{
    if(data->type == LITERAL) *val = data->payload.literal;
    if(data->type == NUMERIC) *val = &data->payload.numeral;
    if(data->type == BOOLEAN) *val = &data->payload.boolean;
    if(data->type == LIST || data->type == PAIR) *val = (dtree*) data->payload.list;
    return SUCCESS;
}


dt_err dtree_free(dtree *data)
{
    if(data == NULL) return SUCCESS;
    if(data->copy == SHALLOW) return NODE_NOT_ORIGINAL;

    if(data->type == LITERAL) {
        if(data->payload.literal) free(data->payload.literal);

    } else if(data->type == LIST || data->type == PAIR) {
        int i;
        dt_err err;
        for(i = 0; i < data->used; i++) {

            err = dtree_free(data->payload.list[i]);
            if(err) return err;
        }

        free(data->payload.list);

    } else if(data->type == POINTER) {
        if(data->copy != SHALLOW && data->payload.pointer)
            free(data->payload.pointer);
    }

    free(data);
    return SUCCESS;
}


dt_err dtree_free_shallow(dtree *data)
{
    if(data == NULL) return SUCCESS;

    if(data->type == LITERAL) {
        if(data->payload.literal) free(data->payload.literal);
    } else if(data->type == LIST || data->type == PAIR) {
        int i;
        dt_err err;
        for(i = 0; i < data->size; i++) {
            err = dtree_free(data->payload.list[i]);
            if(err) return err;
        }

        free(data->payload.list);
    }

    free(data);
    return SUCCESS;
}


const char *dtree_dtype(dtree *data)
{
    switch(data->type) {
        case LITERAL:       return "Literal";
        case NUMERIC:       return "Numeric";
        case BOOLEAN:       return "Boolean";
        case LIST:          return "List";
        case PAIR:          return "Pair";
        case POINTER:       return "Pointer";
        default:            return "Unknown";
    }
}


/**************** PRIVATE UTILITY FUNCTIONS ******************/


/**
 * Steps down the list hirarchy of a dyntree node to
 * find a sub-child target. Returns 0 if it can be found.
 *
 * @param data
 * @param target
 * @return
 */
int list_search(dtree **direct_parent, dtree *data, dtree *target)
{
    /* Check if data is actually valid */
    if(data == NULL) return 1;

    /* Compare the pointers :) */
    if(data == target) return 0;

    int res = 1;
    if(data->type == LIST || data->type == PAIR) {
        int i;
        for(i = 0; i < data->used; i++) {
            res = list_search(direct_parent, data->payload.list[i], target);
            if(res == 0) {

                /* Save the node that contains our child for later */
                (*direct_parent) = data;
                return res;
            }
        }
    }

    return res;
}


/**
 * Small utility function that checks if a datablock is valid to write into.
 * Potentially releases previously owned memory to prevent memory leaks
 *
 * @param data The dtree object to check
 * @return
 */
dt_err data_check(dtree *data)
{
    /* Check if the data block has children */
    if(data->type == LIST)
    {
        printf("Won't override heap payload with data!");
        return INVALID_PAYLOAD;
    }

    /* Free the existing string */
    if(data->type == LITERAL)
    {
        if(data->payload.literal) free(data->payload.literal);
    }

    return SUCCESS;
}
