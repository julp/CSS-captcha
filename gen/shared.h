#ifndef SHARED_H

# define SHARED_H 1

enum {
/*
    TABLE_0 = 0,
    ...
    TABLE_9 = 9,
    TABLE_a = 10,
*/
    TABLE_z = 35,
    TABLE_SPACES, // whitespace and invisible separator
    TABLE_COMBINABLES,
    TABLE_LAST = TABLE_COMBINABLES,
    _TABLE_COUNT
};

enum {
    ASCII,
# define UNICODE_FIRST(M, m, p) \
    UNICODE_FIRST = UNICODE_##M##_##m##_##p,
# define UNICODE_LAST(M, m, p) \
    UNICODE_LAST = UNICODE_##M##_##m##_##p,
# define UNICODE_VERSION(M, m, p) \
    UNICODE_##M##_##m##_##p,
# include "supported_unicode_versions.h"
# undef UNICODE_FIRST
# undef UNICODE_LAST
# undef UNICODE_VERSION
    _UNICODE_VERSIONS_COUNT
};

#endif
