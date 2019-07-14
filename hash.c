#include "bowl.h"
#include "array.h"
#include "hash.h"
#include "utils.h"

#include <malloc.h>

typedef struct {
    char *key;
    struct bowl *data;
} hash_item;

err_t hash_insert_key(struct bowl *self, char *key, struct bowl *child)
{
    CHECK(self, INVALID_STATE)
    CHECK(child, INVALID_STATE)
    CHECK(key, INVALID_STATE)

    // Even though we're a HASH node, we use an array under the hood
    struct bowl_arr *arr = self->_pl.array;
    CHECK(arr, INVALID_STATE)

    size_t idx;
    err_t e = _hash(key, arr->size, &idx);
    if(e) return e;

    // If the index is used, rescale and try again
    if(arr->ptr[idx] != NULL) {
        struct bowl_arr *prev = arr;

        // Allocate new hash array
        e = array_malloc(self, arr->size * 2);
        if(e) return e;

        for(int i = 0; i < arr->size; i++) {
            if(arr->ptr[i] != NULL) {
                // FIXME: This is horrible
                hash_item *item = (hash_item*) arr->ptr[i];
                e = hash_insert_key(self, item->key, item->data);
                if(e) return e;
            }
        }

        e = hash_insert_key(self, key, child);
        if(e) return e;

        hash_free_shallow(prev);

    } else {
        hash_item *item = malloc(sizeof(hash_item));
        CHECK(item, MALLOC_FAILED)

        item->key = calloc(sizeof(char), REAL_STRLEN(key));
        CHECK(item->key, MALLOC_FAILED)
        strcpy(item->key, key);

        item->data = child;

        arr->ptr[idx] = (struct bowl*) item;
        arr->used += 1;
    }

    return OK;
}

err_t hash_remove_key(struct bowl *self, char *key, struct bowl **child)
{
    CHECK(self, INVALID_STATE)
    CHECK(child, INVALID_STATE)
    CHECK(key, INVALID_STATE)

    // Even though we're a HASH node, we use an array under the hood
    struct bowl_arr *arr = self->_pl.array;
    CHECK(arr, INVALID_STATE)

    size_t idx;
    err_t e = _hash(key, arr->size, &idx);
    if(e) return e;

    (*child) = arr->ptr[idx];
    arr->used -= 1;
    arr->ptr[idx] = NULL;
    return OK;
}

err_t hash_free(struct bowl *self)
{
    CHECK(self, INVALID_PARAMS)
    struct bowl_arr *arr = self->_pl.array;
    CHECK(arr, INVALID_PARAMS)

    err_t e = OK;
    for(int i = 0; i < arr->size; i++) {
        hash_item *item = (hash_item *) arr->ptr[i];
        if(item != NULL) {
            CHECK(item->data, INVALID_STATE)
            CHECK(item->key, INVALID_STATE)

            e = bowl_free(item->data);
            if(e) break;

            free(item->key);
            free(item);
        }
    }

    free(arr->ptr);
    free(arr);
    return e;
}

err_t hash_free_shallow(struct bowl_arr *arr)
{
    CHECK(arr, INVALID_PARAMS);
    for(int i = 0; i < arr->size; i++) {
        hash_item *item = (hash_item *) arr->ptr[i];
        if(item != NULL) {
            free(item->key);
            free(item);
        }
    }

    free(arr->ptr);
    free(arr);
    return OK;
}
