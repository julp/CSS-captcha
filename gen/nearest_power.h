#pragma once

#include <limits.h>

#define HALF_SIZE (1UL << (sizeof(size_t) * CHAR_BIT - 1))

static inline CONST size_t nearest_power(size_t requested_length, size_t minimal)
{
    if (requested_length > HALF_SIZE) {
        return HALF_SIZE;
    } else {
        int i = 1;
        requested_length = MAX(requested_length, minimal);
        while ((1UL << i) < requested_length) {
            i++;
        }

        return (1UL << i);
    }
}
