#ifndef HASH_H_
#define HASH_H_

#include "bowl.h"

err_t hash_insert_key(struct bowl *, char *key, struct bowl *);

err_t hash_remove_key(struct bowl *, char *key, struct bowl **);

#endif // HASH_H_

