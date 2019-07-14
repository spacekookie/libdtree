#include "bowl.h"
#include "array.h"
#include "hash.h"

#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>

#define ARRAY_START_SIZE 2
#define HASH_START_SIZE 24

void *_get_data(struct bowl *ptr)
{
    switch(ptr->type) {
        case ARRAY | HASH:  return ptr->_pl.array;
        case LINKED:        return ptr->_pl.linked;
        case LEAF:          return ptr->_pl.data;
        default:            return NULL;
    }
}

err_t bowl_malloc(struct bowl **ptr, bowl_t type)
{
    CHECK((type != 0), INVALID_PARAMS)

    (*ptr) = malloc(sizeof(struct bowl));
    CHECK(*ptr, MALLOC_FAILED)

    memset(*ptr, 0, sizeof(struct bowl));
    (*ptr)->type = type;

    switch((*ptr)->type) {
        case LEAF: return OK; // No further allocation needed
        case ARRAY: return array_malloc(*ptr, ARRAY_START_SIZE);
        default: return INVALID_STATE;
    }

    return OK;
}

err_t bowl_append(struct bowl *self, struct bowl *new)
{
    CHECK(self, INVALID_PARAMS)
    CHECK(new, INVALID_PARAMS)

    switch(self->type) {
        case LEAF: return INVALID_PARAMS;
        case ARRAY: return array_insert(self, new);

        default: return INVALID_STATE;
    }

    return OK;
}

err_t bowl_insert_key(struct bowl *self, char *key, struct bowl *child)
{
    CHECK(self, INVALID_PARAMS)
    CHECK(child, INVALID_PARAMS)
    CHECK(key, INVALID_PARAMS)

    switch(self->type) {
        case HASH: return hash_insert_key(self, key, child);
        default: return INVALID_PARAMS;
    }
}

err_t bowl_insert_idx(struct bowl *self, size_t idx, struct bowl *child)
{
    CHECK(self, INVALID_PARAMS)
    CHECK(child, INVALID_PARAMS)
    CHECK((idx >= 0), INVALID_PARAMS)

    switch(self->type) {
        case LEAF: return INVALID_PARAMS;
        case ARRAY: return array_insert_key(self, idx, child);

        default: return INVALID_STATE;
    }
}

err_t bowl_swap_idx(struct bowl *self, size_t idx, struct bowl *child, struct bowl **prev)
{
    CHECK(self, INVALID_PARAMS)
    CHECK(child, INVALID_PARAMS)
    CHECK((idx >= 0), INVALID_PARAMS)

    switch(self->type) {
        case LEAF: return INVALID_PARAMS;
        case ARRAY: return array_swap_key(self, idx, child, prev);

        default: return INVALID_STATE;
    }
}

err_t bowl_remove(struct bowl *self, struct bowl *child)
{
    CHECK(self, INVALID_PARAMS)
    CHECK(child, INVALID_PARAMS)

    switch(self->type) {
        case LEAF: return INVALID_PARAMS;
        case ARRAY: return array_remove(self, child);

        default: return INVALID_STATE;
    }
}

err_t bowl_remove_key(struct bowl *self, char *key, struct bowl **prev)
{
    CHECK(self, INVALID_PARAMS)
    CHECK(key, INVALID_PARAMS)

    switch(self->type) {
        case LEAF: return INVALID_PARAMS;
        case HASH: return hash_remove_key(self, key, prev);

        default: return INVALID_STATE;
    }
}

err_t bowl_remove_idx(struct bowl *self, size_t idx, struct bowl **prev)
{
    CHECK(self, INVALID_PARAMS)
    CHECK((idx >= 0), INVALID_PARAMS)

    switch(self->type) {
        case LEAF: return INVALID_PARAMS;
        case ARRAY: return array_remove_key(self, idx, prev);

        default: return INVALID_STATE;
    }
}

err_t bowl_free(struct bowl *self)
{
    CHECK(self, INVALID_PARAMS)

    err_t e;
    switch(self->type) {
        case LEAF: e = data_free(self->_pl.data); break;
        case ARRAY | HASH: e = array_free(self); break;
        default: e = INVALID_STATE; break;
    }
    if(e) return e;

    free(self);
    return OK;
}
