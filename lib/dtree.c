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

int recursive_search(dtree**, dtree *, dtree *);

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
    } else if(data->type == RECURSIVE || data->type == PAIR) {

        /* Iterate over all children and clear them */
        int i;
        dt_err err;
        for(i = 0; i < data->size; i++) {
            err = dtree_free(data->payload.recursive[i]);
            if(err) return err;
        }
    }

    /* Set the data type to unset */
    data->type = UNSET;
    data->encset = DYNTREE_ENCODE_NONE;
    data->size = 0;
    data->used = 0;

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
    data->used = sizeof(*ptr);

    return SUCCESS;
}


dt_err dtree_addnumeral(dtree *data, long numeral)
{
    /* Make sure we are a literal or unset data object */
    if(data->type != UNSET)
        if(data->type != NUMERAL) return INVALID_PAYLOAD;

    data->payload.numeral = numeral;
    data->type = NUMERAL;
    data->size = sizeof(int);
    data->used = sizeof(int);
    return SUCCESS;
}


dt_err dtree_addlist(dtree *data, dtree *(*new_data))
{
    /* Make sure we are a literal or unset data object */
    if(data->type != UNSET)
        if(data->type != RECURSIVE) return INVALID_PAYLOAD;

    dt_err err;

    /* This means elements already exist */
    if(data->size > 0) {

        /* Used should never > size */
        if(data->used >= data->size) {
            data->size += RDB_REC_MULTIPLY;

            // TODO Use Realloc
            dtree **tmp = (dtree**) malloc(sizeof(dtree*) * data->size);
            memcpy(tmp, data->payload.recursive, sizeof(dtree*) * data->used);

            /* Free the list WITHOUT the children! */
            free(data->payload.recursive);
            data->payload.recursive = tmp;
        }

    /* This means the data object is new */
    } else {
        dtree **tmp = (dtree**) malloc(sizeof(dtree*) * data->size);
        data->payload.recursive = tmp;
        data->type = RECURSIVE;
        data->used = 0;
        data->size = RDB_REC_DEF_SIZE;
    }

    err = dtree_malloc(new_data);
    if(err) return err;

    /* Reference the slot, assign it, then move our ctr */
    data->payload.recursive[data->used] = *new_data;
    data->used++;

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

    data->payload.recursive = tmp;

    { /* Assign data to new array */
        data->payload.recursive[data->used] = *key;
        data->used++;
        data->payload.recursive[data->used] = *value;
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
    int ret = recursive_search(&dp, data, sp);
    if(ret != 0) return DATA_NOT_RELATED;
    if(dp == NULL) return NODE_NOT_FOUND;

    /* Find the exact recursive reference and remove it */
    int i;
    for(i = 0; i < dp->used; i++) {
        if(dp->payload.recursive[i] == NULL) continue;

        /* Manually remove the entry */
        if(dp->payload.recursive[i] == sp) {
            dp->used--;
            dp->payload.recursive[i] = NULL;
        }
    }

    return SUCCESS;
}


dt_err dtree_merge_trees(dtree *data, dtree *merge)
{
    /* REALLY make sure the type is correct */
    if(data->type == UNSET) return INVALID_PARAMS;
    if(!(data->type == RECURSIVE || data->type == PAIR))
        return INVALID_PAYLOAD;

    /* This means elements already exist */
    if(data->size > 0) {

        /* Used should never > size */
        if(data->used >= data->size) {
            data->size += RDB_REC_MULTIPLY;

            dtree **tmp = (dtree**) malloc(sizeof(dtree*) * data->size);
            memcpy(tmp, data->payload.recursive, sizeof(dtree*) * data->used);

            /* Free the list WITHOUT the children! */
            free(data->payload.recursive);
            data->payload.recursive = tmp;
        }

        /* This means the data object is new */
    } else {
        dtree **tmp = (dtree**) malloc(sizeof(dtree*) * data->size);
        data->payload.recursive = tmp;
        data->type = RECURSIVE;
        data->used = 0;
        data->size = RDB_REC_DEF_SIZE;
    }

    /* Reference the slot, assign it, then move our ctr */
    data->payload.recursive[data->used] = merge;
    data->used++;

    return SUCCESS;
}


dt_err dtree_copy_deep(dtree *data, dtree *(*copy))
{
    if(data == NULL) return INVALID_PARAMS;

    return SUCCESS;
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

        case NUMERAL:
            err = dtree_addnumeral(*copy, data->payload.numeral);
            break;

        case RECURSIVE:
            (*copy)->type = RECURSIVE;
            (*copy)->payload.recursive = (dtree**) malloc(sizeof(dtree*) * data->size);
            memcpy((*copy)->payload.recursive, data->payload.recursive, sizeof(dtree*) * data->used);
            break;

        case PAIR:
            (*copy)->type = PAIR;
            (*copy)->payload.recursive = (dtree**) malloc(sizeof(dtree*) * data->size);
            memcpy((*copy)->payload.recursive, data->payload.recursive, sizeof(dtree*) * data->used);
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

    if(data->type == RECURSIVE|| data->type == PAIR) {

        int i;
        for(i = 0; i < data->used; i++) {
            dt_err err = dtree_search_payload(data->payload.recursive[i], found, payload, type);
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

            case NUMERAL:
                if(data->payload.numeral == (long) payload)
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


void recursive_print(dtree *data, const char *offset)
{
    dt_uni_t type = data->type;

    switch(type) {
        case UNSET:
            printf("[NULL]\n");
            break;
        case LITERAL:
            printf("%s['%s']\n", offset, data->payload.literal);
            break;
        case NUMERAL:
            printf("%s[%lu]\n", offset, data->payload.numeral);
            break;
        case PAIR:
        {
            dt_uni_t k_type = data->payload.recursive[0]->type;
            dt_uni_t v_type = data->payload.recursive[1]->type;

            if(k_type == LITERAL) printf("%s['%s']", offset, data->payload.recursive[0]->payload.literal);
            if(k_type == NUMERAL) printf("%s[%lu]", offset, data->payload.recursive[0]->payload.numeral);

            char new_offset[REAL_STRLEN(offset) + 2];
            strcpy(new_offset, offset);
            strcat(new_offset, "  ");

            if(k_type == RECURSIVE || k_type == PAIR) recursive_print(data->payload.recursive[0], new_offset);

            /* Print the value now */

            if(k_type == LITERAL) printf(" => ['%s']\n", data->payload.recursive[1]->payload.literal);
            if(k_type == NUMERAL) printf(" => [%lu]\n", data->payload.recursive[1]->payload.numeral);

            if(k_type == RECURSIVE || k_type == PAIR) recursive_print(data->payload.recursive[1], new_offset);

            break;
        }

        case RECURSIVE:
        {
            int i;
            printf("%s[RECURSIVE]\n", offset);
            for(i = 0; i < data->used; i++) {
                dt_uni_t t = data->payload.recursive[i]->type;


                char new_offset[REAL_STRLEN(offset) + 2];
                strcpy(new_offset, offset);
                strcat(new_offset, "  ");

                if(t == LITERAL || t == NUMERAL) {
                    recursive_print(data->payload.recursive[i], new_offset);
                    continue;
                }

                if(t == RECURSIVE)
                {
                    recursive_print(data->payload.recursive[i], new_offset);
                    continue;
                }

                if(t == PAIR) {
                    printf("%s[PAIR] <==> ", new_offset);
                    recursive_print(data->payload.recursive[i], new_offset);
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
    recursive_print(data, "");
}

dt_err dtree_get(dtree *data, void *(*val))
{
    if(data->type == LITERAL) *val = (char*) data->payload.literal;
    if(data->type == NUMERAL) *val = (int*) &data->payload.numeral;
    if(data->type == RECURSIVE || data->type == PAIR)
        *val = (dtree*) data->payload.recursive;

    return SUCCESS;
}


dt_err dtree_free(dtree *data)
{
    if(data == NULL) return SUCCESS;

    if(data->type == LITERAL) {
        if(data->payload.literal) free(data->payload.literal);

    } else if(data->type == RECURSIVE || data->type == PAIR) {
        int i;
        dt_err err;
        for(i = 0; i < data->used; i++) {
            err = dtree_free(data->payload.recursive[i]);
            if(err) return err;
        }

        free(data->payload.recursive);

    } else if(data->type == POINTER) {
        if(data->payload.pointer) free(data->payload.pointer);
    }

    free(data);
    return SUCCESS;
}


dt_err dtree_free_shallow(dtree *data)
{
    if(data == NULL) return SUCCESS;

    if(data->type == LITERAL) {
        if(data->payload.literal) free(data->payload.literal);
    } else if(data->type == RECURSIVE || data->type == PAIR) {
        int i;
        dt_err err;
        for(i = 0; i < data->size; i++) {
            err = dtree_free(data->payload.recursive[i]);
            if(err) return err;
        }

        free(data->payload.recursive);
    }

    free(data);
    return SUCCESS;
}


const char *dtree_dtype(dtree *data)
{
    switch(data->type) {
        case LITERAL:       return "Literal";
        case NUMERAL:       return "Numeral";
        case RECURSIVE:     return "Recursive";
        case PAIR:          return "Pair";
        case POINTER:       return "Pointer";
        default:            return "Unknown";
    }
}

/**************** PRIVATE UTILITY FUNCTIONS ******************/

/**
 * Steps down the recursive hirarchy of a dyntree node to
 * find a sub-child target. Returns 0 if it can be found.
 *
 * @param data
 * @param target
 * @return
 */
int recursive_search(dtree **direct_parent, dtree *data, dtree *target)
{
    /* Check if data is actually valid */
    if(data == NULL) return 1;

    /* Compare the pointers :) */
    if(data == target) return 0;

    int res = 1;
    if(data->type == RECURSIVE || data->type == PAIR) {
        int i;
        for(i = 0; i < data->used; i++) {
            res = recursive_search(direct_parent, data->payload.recursive[i], target);
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
    if(data->type == RECURSIVE)
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
