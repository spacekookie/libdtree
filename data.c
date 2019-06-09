#include "bowl.h"

#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>
#include <stdarg.h>

/// Allocate memory for a new data node
err_t data_malloc(struct bowl **self, data_t type, ...)
{
    err_t e;
    va_list args;

    struct data *d = calloc(sizeof(struct data), 1);
    CHECK(d, MALLOC_FAILED)
    d->type = type;

    va_start(args, type);
    switch(type) {
        case LITERAL: {
            char *lit = va_arg(args, char *);
            size_t len = REAL_STRLEN(lit);

            d->_pl.literal = calloc(sizeof(char), len);
            if(!d->_pl.literal) {
                e = MALLOC_FAILED;
                goto fail;
            }

            strcpy(d->_pl.literal, lit);
            break;
        };
        case INTEGER: {
            int i = va_arg(args, long);
            d->_pl.integer = i;
            break;
        }
        case REAL: {
            int r = va_arg(args, double);
            d->_pl.real = r;
            break;
        }
        case BOOLEAN: {
            int b = va_arg(args, int);
            d->_pl.boolean = (bool) b;
            break;
        }
        case RAW: {
            void *raw = va_arg(args, void *);
            d->_pl.raw = raw;
            break;
        }
        default: e = INVALID_PARAMS; goto fail;
    }
    va_end(args);

    e = bowl_malloc(self, LEAF);
    if(e) goto fail;

    (*self)->_pl.data = d;
    return OK;

fail:
    if(d->type == LITERAL && d->_pl.literal) free(d->_pl.literal);
    if(*self) bowl_free(*self);
    if(d) free(d);
    return e;
}

/// Free all data node memory
err_t data_free(struct data *self)
{
    CHECK(self, INVALID_PARAMS)
    switch(self->type) {
        case LITERAL: free(self->_pl.literal); break;
        default: break;
    }

    free(self);
    return OK;
}
