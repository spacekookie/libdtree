
/* Make sure we're not included multiple times */
#ifndef _DYN_ERR_H
#define _DYN_ERR_H

/* Also make sure we're _always_ interpreted as a C file */
#ifdef __cplusplus
extern "C" {
#endif

/** Define some generic error codes first that we can propagate **/
typedef enum dt_err {

    /* General purpose error codes */
    FAILURE = -1,
    SUCCESS = 0,

    INVALID_PARAMS,
    MALLOC_FAILED,
    INVALID_PAYLOAD

} dt_err;

const char *rdb_error_getmsg(dt_err *e);

#ifdef __cplusplus
}
#endif
#endif /* _DYN_ERR_H */