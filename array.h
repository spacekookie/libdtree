#ifndef _ARRAY_H_
#define _ARRAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bowl.h"

err_t array_malloc(struct bowl *, size_t);

err_t array_insert(struct bowl *, struct bowl *);

err_t array_insert_key(struct bowl *, size_t idx, struct bowl *);

err_t array_swap_key(struct bowl *, size_t idx, struct bowl *, struct bowl **);

err_t array_remove(struct bowl *, struct bowl *);

err_t array_remove_key(struct bowl *, size_t, struct bowl **);

err_t array_free(struct bowl *);

err_t array_free_shallow(struct bowl_arr *);

#ifdef __cplusplus
}
#endif
#endif // _ARRAY_H_
