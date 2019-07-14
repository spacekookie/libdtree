#include "bowl.h"
#include "array.h"
#include "utils.h"

#include <malloc.h>


err_t array_malloc(struct bowl *self, size_t size)
{
    CHECK(self, INVALID_STATE)
    CHECK((size > 0), INVALID_PARAMS)

    struct bowl_arr *arr = malloc(sizeof(struct bowl_arr));
    CHECK(arr, MALLOC_FAILED)
    arr->size = size;
    arr->used = 0;

    struct bowl **ptr = calloc(sizeof(struct bowl *), arr->size);
    CHECK(ptr, MALLOC_FAILED)
    arr->ptr = ptr;

    self->_pl.array = arr;
    return OK;
}

err_t array_insert(struct bowl *self, struct bowl *new)
{
    CHECK(self, INVALID_PARAMS)
    CHECK(new, INVALID_PARAMS)

    struct bowl_arr *arr = self->_pl.array;
    CHECK(arr, INVALID_STATE)

    err_t e = _array_rescale((void ***) &arr->ptr, &arr->size, arr->used);
    if(e) return e;

    arr->ptr[arr->used++] = new;
    return OK;
}

err_t array_insert_key(struct bowl *self, size_t idx, struct bowl *new)
{
    CHECK(self, INVALID_PARAMS)
    CHECK(new, INVALID_PARAMS)

    struct bowl_arr *arr = self->_pl.array;
    CHECK(arr, INVALID_STATE)
    CHECK((idx < arr->used), INVALID_PARAMS)

    err_t e = _array_rescale((void ***) &arr->ptr, &arr->size, arr->used);
    if(e) return e;

    for(int i = idx + 1; idx < arr->used; i++)
        arr->ptr[i] = arr->ptr[i - 1];

    arr->ptr[idx] = new;
    arr->used++;

    return OK;
}

err_t array_swap_key(struct bowl *self, size_t idx, struct bowl *new, struct bowl **old)
{
    CHECK(self, INVALID_PARAMS)
    CHECK(new, INVALID_PARAMS)

    struct bowl_arr *arr = self->_pl.array;
    CHECK(arr, INVALID_STATE)

    (*old) = NULL; // Explicitly set to NULL if no such key
    CHECK((idx < arr->used), INVALID_PARAMS)

    (*old) = arr->ptr[idx];
    arr->ptr[idx] = new;

    return OK;
}

err_t array_remove(struct bowl *self, struct bowl *to_remove)
{
    CHECK(self, INVALID_PARAMS)
    CHECK(to_remove, INVALID_PARAMS)

    struct bowl_arr *arr = self->_pl.array;
    CHECK(arr, INVALID_STATE)

    size_t idx;
    err_t e = _array_search((void **) arr->ptr, arr->size, &idx, to_remove);
    if(e) return e;

    e = _array_remove((void **) arr->ptr, idx, arr->size, NULL);
    return e;
}

err_t array_remove_key(struct bowl *self, size_t idx, struct bowl **out)
{
    CHECK(self, INVALID_PARAMS);
    struct bowl_arr *arr = self->_pl.array;
    CHECK(arr, INVALID_STATE)
    CHECK((idx < arr->used), INVALID_PARAMS)

    err_t e = _array_remove((void **) arr->ptr, idx, arr->size, (void **) out);
    return e;
}

err_t array_free(struct bowl *self)
{
    CHECK(self, INVALID_PARAMS)
    struct bowl_arr *arr = self->_pl.array;
    CHECK(arr, INVALID_STATE)

    err_t e;
    for(int i = 0; i < arr->used; i++) {
        e = bowl_free(arr->ptr[i]);
        if(e) break;
    }

    free(arr->ptr);
    free(arr);
    return e;
}

err_t array_free_shallow(struct bowl_arr *arr)
{
    CHECK(arr, INVALID_STATE)
    free(arr->ptr);
    free(arr);
    return OK;
}
