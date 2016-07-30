// Include our header file
#include <dtree/dyn_tree.h>

// Runtime includes
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <stdbool.h>

#define RDB_REC_DEF_SIZE    2
#define RDB_REC_MULTIPLY    2
#define REAL_STRLEN(str) (strlen(str) + 1)

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
    data->size = 0;
    data->used = 0;

    return SUCCESS;
}

dt_err dtree_addliteral(dtree *data, const char *literal, size_t length)
{
    /* Make sure we are a literal or unset data object */
    if(data->type != UNSET)
        if(data->type != LITERAL) return INVALID_PAYLOAD;

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

dt_err dtree_addnumeral(dtree *data, int numeral)
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

dt_err dtree_addrecursive(dtree *data, dtree *(*new_data))
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
            printf("%s[%d]\n", offset, data->payload.numeral);
            break;
        case PAIR:
            {
                dt_uni_t k_type = data->payload.recursive[0]->type;
                dt_uni_t v_type = data->payload.recursive[1]->type;

                if(k_type == LITERAL) printf("%s['%s']", offset, data->payload.recursive[0]->payload.literal);
                if(k_type == NUMERAL) printf("%s[%d]", offset, data->payload.recursive[0]->payload.numeral);

                char new_offset[REAL_STRLEN(offset) + 2];
                strcpy(new_offset, offset);
                strcat(new_offset, "  ");

                if(k_type == RECURSIVE || k_type == PAIR) recursive_print(data->payload.recursive[0], new_offset);

                /* Print the value now */

                if(k_type == LITERAL) printf(" => ['%s']\n", data->payload.recursive[1]->payload.literal);
                if(k_type == NUMERAL) printf(" => [%d]\n", data->payload.recursive[1]->payload.numeral);

                if(k_type == RECURSIVE || k_type == PAIR) recursive_print(data->payload.recursive[1], new_offset);
            }
            break;

        case RECURSIVE:
        {
            int i;
            printf("%s[RECURSIVE]\n", offset);
            for(i = 0; i < data->used; i++) {
                dt_uni_t type = data->payload.recursive[i]->type;


                char new_offset[REAL_STRLEN(offset) + 2];
                strcpy(new_offset, offset);
                strcat(new_offset, "  ");

                if(type == LITERAL || type == NUMERAL) {
                    recursive_print(data->payload.recursive[i], new_offset);
                    continue;
                }

                if(type == RECURSIVE)
                {
                    recursive_print(data->payload.recursive[i], new_offset);
                    continue;
                }

                if(type == PAIR) {
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
        for(i = 0; i < data->size; i++) {
            err = dtree_free(data->payload.recursive[i]);
            if(err) return err;
        }

        free(data->payload.recursive);
    }

    free(data);
    return SUCCESS;
}

const char *dtree_dtype(dt_uni_t type)
{
    switch(type) {
        case LITERAL: return "Literal";
        case NUMERAL: return "Numeral";
        case RECURSIVE: return "Recursive";
        default: return "Unknown";
    }
}

/**************** PRIVATE UTILITY FUNCTIONS ******************/


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
