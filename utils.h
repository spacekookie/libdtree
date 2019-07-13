#ifndef _UTIL_H_
#define _UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bowl.h"

err_t _array_rescale(void ***, size_t *len, size_t use);

err_t _array_search(void **, size_t, size_t *out, void *in);

err_t _array_remove(void **, size_t idx, size_t len, void **out);

err_t _hash(char *str, size_t len, size_t *out);

#ifdef __cplusplus
}
#endif
#endif // _UTIL_H_
