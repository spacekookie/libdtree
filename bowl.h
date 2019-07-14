#ifndef _BOWL_H_
#define _BOWL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <memory.h>
#include <stdbool.h>

#define REAL_STRLEN(str) (strlen(str) + 1)
#define CHECK(ptr, err) if(!ptr) return err;

/** Define some generic error codes first that we can propagate **/
typedef enum {
    OK = 0,
    ERR = -1,
    NOT_IMPLEMENTED = 1,    // A runtime error for missing features

    INVALID_PARAMS = 2,     // A function didn't get the required parameters
    MALLOC_FAILED = 3,      // A memory allocation failed
    INVALID_STATE = 4,      // Calling an operation on an invalid state
} err_t;

typedef enum {
    LITERAL,    // Any string that is \0 terminated
    INTEGER,    // Integer value
    REAL,       // Floating point value
    BOOLEAN,    // A 1-byte boolean
    RAW         // A platform-length pointer
} data_t;

typedef enum {
    LEAF = 1, ARRAY, HASH, LINKED
} bowl_t;

struct bowl {
    bowl_t  type;
    union {
        struct data     *data;
        struct bowl_arr *array;
        struct b_ptr    *linked;
    } _pl;
};

struct data {
    data_t type;
    union {
        char    *literal;
        bool    boolean;
        long    integer;
        double  real;
        void    *raw;
    } _pl;
};

struct bowl_arr {
    size_t size, used;
    struct bowl **ptr;
};

struct b_ptr {
    struct bowl     *self;
    struct b_ptr    *next;
};

/// Allocate memory for an new, empty bowl node
err_t bowl_malloc(struct bowl **, bowl_t);

/// Insert one node at the end of another
err_t bowl_append(struct bowl *, struct bowl *);

/// Insert a node under a specific key (relevant for HASH nodes)
err_t bowl_insert_key(struct bowl *, char *key, struct bowl *);

/// Insert a node into a specific index of another node
err_t bowl_insert_idx(struct bowl *, size_t idx, struct bowl *);

/// Insert and swap a key in place, returning the old one
err_t bowl_swap_idx(struct bowl *, size_t idx, struct bowl *, struct bowl **);

/// Remove a bowl node by it's pointer reference
err_t bowl_remove(struct bowl *, struct bowl *);

/// Remove a specific key (relevant for HASH nodes)
err_t bowl_remove_key(struct bowl *, char *key, struct bowl **);

/// Removing a bowl node with a key
err_t bowl_remove_idx(struct bowl *, size_t idx, struct bowl **);

/// Cascade-free memory from a bowl node
err_t bowl_free(struct bowl *);

/// Allocate memory for a new data node
err_t data_malloc(struct bowl **, data_t, ...);

/// Free all data node memory
err_t data_free(struct data *);

#ifdef __cplusplus
}
#endif
#endif // _BOWL_H_
