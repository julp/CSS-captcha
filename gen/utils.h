#pragma once

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#define STR_LEN(str)      (ARRAY_SIZE(str) - 1)
#define STR_SIZE(str)     (ARRAY_SIZE(str))

#ifndef MAX
# define MAX(a, b) ({ typeof (a) _a = (a); typeof (b) _b = (b); _a > _b ? _a : _b; })
#endif /* !MAX */

#ifndef MIN
# define MIN(a, b) ({ typeof (a) _a = (a); typeof (b) _b = (b); _a < _b ? _a : _b; })
#endif /* !MIN */

#define HAS_FLAG(value, flag) \
    (0 != ((value) & (flag)))

#define SET_FLAG(value, flag) \
    ((value) |= (flag))

#define UNSET_FLAG(value, flag) \
    ((value) &= ~(flag))
