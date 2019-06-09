#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

#define DELTA       0x2
#define OVERSHOOT   0x8

err_t _array_rescale(void ***ptr, size_t *len, size_t use)
{
    CHECK(len, INVALID_PARAMS)
    CHECK((use <= *len), INVALID_PARAMS)
    CHECK(ptr, INVALID_PARAMS)

    // This is a simple linear scale strategy
    if ((int) (*len - OVERSHOOT) >= (int) (use + DELTA))  *len -= OVERSHOOT;
    if (use + DELTA >= *len)                              *len += DELTA;

    *ptr = realloc(*ptr, *len * sizeof(void *));
    return OK;
}

err_t _array_search(void **ptr, size_t len, size_t *out, void *in)
{
    CHECK(ptr, INVALID_PARAMS)
    CHECK(in, INVALID_PARAMS)
    CHECK((len > 0), INVALID_PARAMS)
    (*out) = -1;

    for(int i = 0; i < len; i++)
        if((*ptr) == in) (*out) = i;

    return OK;
}

err_t _array_remove(void **ptr, size_t idx, size_t len, void **out)
{
    CHECK(ptr, INVALID_PARAMS)
    CHECK((idx >= 0), INVALID_PARAMS)
    CHECK((len > idx), INVALID_PARAMS)

    if(out != NULL) (*out) = ptr[idx];
    for(int i = idx + 1; idx < len; i++)
        ptr[i - 1] = ptr[i];

    return OK;
}
