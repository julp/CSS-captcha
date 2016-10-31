/**
 * @file dynamic_arrays/darray.c
 * @brief dynamic array of elements of any size (but all elements have to be of the same size)
 *
 * Example to store integers:
 * \code
 *   size_t i;
 *   DArray da;
 *   int values[] = { 3, 67, 24, 18 };
 *
 *   darray_init(&da, sizeof(int));
 *   darray_append_all(&da, values, ARRAY_SIZE(values));
 *   darray_sort(&da, intcmp, NULL);
 *   for (i = 0; i < darray_size(&da); i++) {
 *       int j;
 *
 *       j = darray_at_unsafe(&da, i, int);
 *       printf("%zu: %d\n", i, j);
 *   }
 *   darray_destroy(&da);
 * \endcode
 *
 * A DArray is also iterable. The last loop could also be written:
 * \code
 *   Iterator it;
 *
 *   darray_to_iterator(&it, &da);
 *   for (i = 0, iterator_first(&it); iterator_is_valid(&it); i++, iterator_next(&it)) {
 *       int *j;
 *
 *       j = iterator_current(&it);
 *       printf("%zu: %d\n", i, *j);
 *   }
 *   iterator_close(&it);
 * \endcode
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "attributes.h"
#include "utils.h"
#include "darray.h"
#include "nearest_power.h"

#define DARRAY_INCREMENT 8
#define DARRAY_MIN_LENGTH 16U

#define OFFSET_TO_ADDR(/*DArray **/ da, /*unsigned int*/ offset) \
    ((da)->data + (da)->element_size * (offset))

#define LENGTH(/*DArray **/ da, /*size_t*/ count) \
    ((da)->element_size * (count))

// post-deletion
// NOTE: have to be called AFTER da->length has been decremented
static inline void darray_wipeout(DArray *da, size_t count)
{
//     if (NULL == da->default_value) {
        bzero(OFFSET_TO_ADDR(da, da->length), LENGTH(da, count));
//     } else {
//         size_t i;
//
//         for (i = 0; i < count; i++) {
//             memcpy(OFFSET_TO_ADDR(da, da->length - i), da->default_value, da->element_size);
//         }
//     }
}

// pre-deletion
static inline void darray_destroy_elements(DArray *da, unsigned int from, size_t count)
{
    if (NULL != da->dtor) {
        size_t i;

        for (i = 0; i < count; i++) {
            da->dtor((void *) OFFSET_TO_ADDR(da, from + i));
        }
    }
}

static inline void darray_maybe_resize_to(DArray *da, size_t total_length)
{
    assert(NULL != da);

    if (UNEXPECTED(total_length >= da->allocated)) {
        da->allocated = ((total_length / da->capacity_increment) + 1) * da->capacity_increment;
        da->data = realloc(da->data, da->element_size * da->allocated);
        darray_wipeout(da, da->allocated - da->length);
    }
}

static inline void darray_maybe_resize_of(DArray *da, size_t additional_length)
{
    darray_maybe_resize_to(da, da->length + additional_length);
}

/**
 * Initialize a dynamic array with custom attributes
 *
 * @param da the dynamic array
 * @param element_size the size, in bytes, requested to store a single element
 * @param initial_capacity the initial space to allocate from the start
 * @param capacity_increment the capacity increment for array groths when there is no more space
 **/
void darray_init_custom(DArray *da, DtorFunc dtor, size_t element_size, size_t initial_capacity, size_t capacity_increment)
{
    da->data = NULL;
    da->dtor = dtor;
//     da->default_value = NULL;
    da->length = da->allocated = 0;
    da->element_size = element_size;
    da->capacity_increment = nearest_power(capacity_increment, 2);
    darray_maybe_resize_to(da, MIN(DARRAY_MIN_LENGTH, initial_capacity));
}

/**
 * Initialize a dynamic array with internal defaults
 *
 * @param da the dynamic array to initialize
 * @param element_size the size, in bytes, needed to store an element
 **/
void darray_init(DArray *da, DtorFunc dtor, size_t element_size)
{
    darray_init_custom(da, dtor, element_size, DARRAY_MIN_LENGTH, DARRAY_INCREMENT);
}

/**
 * Destroy a dynamic array
 *
 * @param da the dynamic array to internally free
 **/
void darray_destroy(DArray *da)
{
    darray_destroy_elements(da, 0, da->length);
    free(da->data);
    da->data = NULL;
}

/**
 * Clear a dynamic array for reuse.
 * Wipe out (bzero) any stored values and reset its length to 0.
 * Its capacity is unchanged (not lowered).
 *
 * @param da the dynamic array to reset
 **/
void darray_clear(DArray *da)
{
    darray_destroy_elements(da, 0, da->length);
    darray_wipeout(da, da->length);
    da->length = 0;
}

/**
 * Copy the value from an index to an address
 * "Similar" to value = da[offset]
 *
 * @param da the source dynamic array
 * @param offset the index of the value to retrieve
 * @param value the location where to copy the data
 *
 * @return false if offset is out of bound
 **/
bool darray_at(DArray *da, unsigned int offset, void *value)
{
    if (offset < da->length) {
        memcpy(value, OFFSET_TO_ADDR(da, offset), LENGTH(da, 1));
        return true;
    } else {
        return false;
    }
}

/**
 * A specialized function to retrieve and delete at once the first value of a dynamic array
 * Deletion is done by shifting values of one index
 *
 * @param da the dynamic array
 * @param value the location where to copy the value
 *
 * @return false if there is no more value to fetch
 **/
bool darray_shift(DArray *da, void *value)
{
    if (da->length > 0) {
        darray_destroy_elements(da, 0, 1);
        memcpy(value, OFFSET_TO_ADDR(da, 0), LENGTH(da, 1));
        if (--da->length > 0) {
            memmove(OFFSET_TO_ADDR(da, 0), OFFSET_TO_ADDR(da, 1), LENGTH(da, da->length));
        }
        darray_wipeout(da, 1);
        return true;
    } else {
        return false;
    }
}

/**
 * A specialized function to fetch and delete at once the last value of a dynamic array
 *
 * @param da the dynamic array to alter
 * @param value the location where to copy the value
 *
 * @return false if there is nothing to retrieve
 **/
bool darray_pop(DArray *da, void *value)
{
    if (da->length > 0) {
        darray_destroy_elements(da, da->length/* XXX: -1 ? */, 1);
        memcpy(value, OFFSET_TO_ADDR(da, --da->length), LENGTH(da, 1));
        darray_wipeout(da, 1);
        return true;
    } else {
        return false;
    }
}

/**
 * Append one or more values to an array
 *
 * @param da the dynamic array
 * @param data an array of values to append
 * @param data_count the number of values
 **/
void darray_append_all(DArray *da, const void * const data, size_t data_count)
{
    darray_maybe_resize_of(da, data_count);
    memcpy(OFFSET_TO_ADDR(da, da->length), data, LENGTH(da, data_count));
    da->length += data_count;
}

/**
 * Preprend one or more values to an array
 * Before inserting them, actual values, if any, are moved of *data_count*
 *
 * @param da the dynamic array
 * @param data an array of values to prepend
 * @param data_count the number of values to insert
 **/
void darray_prepend_all(DArray *da, const void * const data, size_t data_count)
{
    darray_maybe_resize_of(da, data_count);
    if (da->length > 0) {
        memmove(OFFSET_TO_ADDR(da, data_count), OFFSET_TO_ADDR(da, 0), LENGTH(da, da->length));
    }
    memcpy(OFFSET_TO_ADDR(da, 0), data, LENGTH(da, data_count));
    da->length += data_count;
}

/**
 * Insert one or more values in an array at a given index
 * Values at index >= *offset* are moved first of *data_count*
 *
 * @param da the dynamic array
 * @param offset the index where to insert the first value of *data*
 * @param data an array of values to insert into *da*
 * @param data_count the number of values into *data*
 **/
void darray_insert_all(DArray *da, unsigned int offset, const void * const data, size_t data_count)
{
    assert(offset <= da->length);

    darray_maybe_resize_of(da, 1);
    if (offset != da->length) {
        memmove(OFFSET_TO_ADDR(da, offset + 1), OFFSET_TO_ADDR(da, offset), LENGTH(da, da->length - offset));
    }
    memcpy(OFFSET_TO_ADDR(da, offset), data, LENGTH(da, data_count));
    da->length += data_count;
}

/**
 * Remove a single value by shifting values after offset of one
 *
 * @param da the dynamic array to modify
 * @param offset the location of the value to squash
 *
 * @return false if offset is out of bound
 **/
bool darray_remove_at(DArray *da, unsigned int offset)
{
    if (offset < da->length) {
        darray_destroy_elements(da, offset, 1);
        memmove(OFFSET_TO_ADDR(da, offset), OFFSET_TO_ADDR(da, offset + 1), LENGTH(da, da->length - offset - 1));
        da->length--;
        darray_wipeout(da, 1);
        return true;
    } else {
        return false;
    }
}

/**
 * Remove (overwrite) values from a range ([*from*;*to*])
 * Values located after *to* are shifted to *from*
 *
 * @param da the dynamic array
 * @param from the lower bound of the range
 * @param to the upper bound of the interval
 **/
void darray_remove_range(DArray *da, unsigned int from, unsigned int to)
{
    size_t diff;

    assert(from < da->length);
    assert(to < da->length);
    assert(from <= to);

    diff = to - from + 1;
    darray_destroy_elements(da, from, diff);
    memmove(OFFSET_TO_ADDR(da, to + 1), OFFSET_TO_ADDR(da, from), LENGTH(da, da->length - diff));
    da->length -= diff;
    darray_wipeout(da, diff);
}

/**
 * Swap 2 values
 *
 * @param da the dynamic array to work on
 * @param offset1 the index of the first of the two values to swap
 * @param offset2 the location of the second one
 **/
void darray_swap(DArray *da, unsigned int offset1, unsigned int offset2)
{
    int i;
    uint8_t tmp, *fp, *tp;

    assert(offset1 != offset2);
    assert(offset1 < da->length);
    assert(offset2 < da->length);

    fp = OFFSET_TO_ADDR(da, offset1);
    tp = OFFSET_TO_ADDR(da, offset2);
    for (i = da->element_size; i > 0; i--) {
        tmp = *fp;
        *fp++ = *tp;
        *tp++ = tmp;
    }
}

/**
 * Set array capacity
 *
 * If *length* < actual capacity, current values above *length* are lost, the dynamic array is not shrinked.
 * If *length* > actual capacity, capacity grows.
 *
 * @param da the dynamic array to act on
 * @param length the new willed length
 **/
void darray_set_size(DArray *da, size_t length)
{
    if (length < da->length) {
        size_t diff;

        diff = da->length - length;
        darray_destroy_elements(da, length, diff);
        da->length = length;
        darray_wipeout(da, diff);
    } else {
        darray_maybe_resize_to(da, length);
    }
}

/**
 * Get the number of elements in a dynamic array
 *
 * @param da the dynamic array
 *
 * @return its length
 **/
size_t darray_length(DArray *da)
{
    return da->length;
}

/**
 * Sort a dynamic array
 *
 * @param da the dynamic array to sort
 * @param cmpfn the callback to compare elements 2-by-2, returns 0 if values are equal,
 *   < 0 if first value is < to the second one and > 0 if first value is > to the second one
 * @param arg a user data to provide to the callback (set it to NULL if unused)
 **/
void darray_sort(DArray *da, CmpFuncArg cmpfn, void *arg)
{
    assert(NULL != da);
    assert(NULL != cmpfn);

    QSORT_R(da->data, da->length, da->element_size, cmpfn, arg);
}

#ifndef WITHOUT_ITERATOR
static void darray_iterator_first(const void *collection, void **state)
{
    DArray *ary;

    assert(NULL != collection);
    assert(NULL != state);

    ary = (DArray *) collection;
    *(uint8_t **) state = ary->data;
}

static void darray_iterator_last(const void *collection, void **state)
{
    DArray *ary;

    assert(NULL != collection);
    assert(NULL != state);

    ary = (DArray *) collection;
    *(uint8_t **) state = ary->data + LENGTH(ary, ary->length - 1);
}

static bool darray_iterator_is_valid(const void *collection, void **state)
{
    DArray *ary;

    assert(NULL != collection);
    assert(NULL != state);

    ary = (DArray *) collection;

    return *((uint8_t **) state) >= ary->data && *((uint8_t **) state) < (ary->data + LENGTH(ary, ary->length));
}

static void darray_iterator_current(const void *collection, void **state, void **key, void **value)
{
    DArray *ary;

    assert(NULL != collection);
    assert(NULL != state);

    ary = (DArray *) collection;
    if (NULL != value) {
        *value = *(uint8_t **) state;
    }
    if (NULL != key) {
        *((uint64_t *) key) = *((uint8_t **) state) - ary->data;
    }
}

static void darray_iterator_next(const void *collection, void **state)
{
    DArray *ary;

    assert(NULL != collection);
    assert(NULL != state);

    ary = (DArray *) collection;
    *((uint8_t **) state) += ary->element_size;
}

static void darray_iterator_previous(const void *collection, void **state)
{
    DArray *ary;

    assert(NULL != collection);
    assert(NULL != state);

    ary = (DArray *) collection;
    *((uint8_t **) state) -= ary->element_size;
}

/**
 * Initialize an *Iterator* to loop, in both directions, on the values of a dynamic array
 *
 * @param it the iterator to initialize
 * @param da the dynamic array to traverse
 *
 * @note iterator directions: forward and backward
 * @note keys (element's index) are typed as uint64_t
 **/
void darray_to_iterator(Iterator *it, DArray *da)
{
    iterator_init(
        it, da, NULL,
        darray_iterator_first, darray_iterator_last,
        darray_iterator_current,
        darray_iterator_next, darray_iterator_previous,
        darray_iterator_is_valid,
        NULL
    );
}
#endif /* !WITHOUT_ITERATOR */
