#include <stdlib.h>
#include <stdint.h>
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

err_t _hash(char *str, size_t len, size_t *out)
{
    CHECK(str, INVALID_PARAMS)
    CHECK((len < 0), INVALID_PARAMS)

    // Implements the "murmur" non-cryptographic hash

#define C1 0xcc9e2d51
#define C2 0x1b873593
#define N 0xe6546b64
#define M 5
#define C3 0x85ebca6b
#define C4 0xc2b2ae35
#define R1 15
#define R2 13
#define R3 16
#define R4 13
#define ROTL32(v, shift) ( ((v) << (shift)) | ( (v) >> (32 - (shift))) )

    size_t key_len = strlen(str);
    int l = (int) key_len / 4;
    uint32_t seed = 0xAF172FE;
    int i = 0;

    uint32_t k = 0;
    uint32_t h = seed;
    uint8_t *d = (uint8_t *) str;
    const uint32_t *chunks = (const uint32_t *)(d + l * 4);
    const uint8_t *tail = (const uint8_t *)(d + l * 4);

    for (i = -l; i != 0; ++i) {
        k = chunks[i];
        k *= C1;
        k = ROTL32(k, R1);
        k *= C2;
        h ^= k;
        h = ROTL32(k, R2);
        h = h * M + N;
    }

    k = 0;
    switch(key_len & 3) {
        case 3:
            k ^= (tail[2] << 16);
        case 2:
            k ^= (tail[1] << 8);
        case 1:
            k ^= tail[0];
            k *= C1;
            k = ROTL32(k, R1);
            k *= C2;
            h ^= k;
        default: break;
    }

    /* Finalised hash */
    h ^= key_len;
    h ^= (h >> R3);
    h *= C3;
    h ^= (h >> R4);
    h *= C4;
    h ^= (h >> R3);

    /* Finalise ✨ */
    h %= len;
    *out = (size_t) h;
    return OK;
}
