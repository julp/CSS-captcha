#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "darray.h"

#define DARRAY_INCREMENT 8
#define DARRAY_MIN_LENGTH 16

#define OFFSET_TO_ADDR(/*DArray **/ da, /*size_t*/ offset) \
    ((da)->data + (da)->element_size * (offset))

#define LENGTH(/*DArray **/ da, /*size_t*/ count) \
    ((da)->element_size * (count))


#define mem_new(type)           malloc((sizeof(type)))
#define mem_new_n(type, n)      malloc((sizeof(type) * (n)))
#define mem_new_n0(type, n)     calloc((n), (sizeof(type)))
#define mem_renew(ptr, type, n) realloc((ptr), (sizeof(type) * (n)))

static void darray_maybe_resize_to(DArray *da, size_t total_length)
{
    assert(NULL != da);

    if (total_length >= da->allocated) {
        da->allocated = ((total_length / DARRAY_INCREMENT) + 1) * DARRAY_INCREMENT;
        da->data = realloc(da->data, da->element_size * da->allocated);
    }
}

static void darray_maybe_resize_of(DArray *da, size_t additional_length)
{
    assert(NULL != da);

    darray_maybe_resize_to(da, da->length + additional_length);
}

// call dtor on [from;to[
static void darray_maybe_dtor(DArray *da, size_t from, size_t to)
{
    assert(NULL != da);

    if (NULL != da->dtor_func) {
        size_t i;

        for (i = from; i < to; i++) {
            da->dtor_func(*(void **) OFFSET_TO_ADDR(da, i));
        }
    }
}

DArray *darray_sized_new(DtorFunc dtor_func, int32_t length, size_t element_size)
{
    DArray *da;

    da = mem_new(*da);
    da->data = NULL;
    da->dtor_func = dtor_func;
    da->element_size = element_size;
    if (length > 0) {
        da->length = da->allocated = 0;
        darray_maybe_resize_to(da, (size_t) length);
    } else {
        da->length = 0;
        da->allocated = DARRAY_MIN_LENGTH;
        da->data = malloc(da->element_size * da->allocated);
    }

    return da;
}

DArray *darray_new(DtorFunc dtor_func, size_t element_size)
{
    return darray_sized_new(dtor_func, DARRAY_MIN_LENGTH, element_size);
}

void darray_destroy(DArray *da)
{
    darray_maybe_dtor(da, 0, da->length);
    free(da->data);
    free(da);
}

void darray_clear(DArray *da)
{
    darray_maybe_dtor(da, 0, da->length);
    da->length = 0;
}

bool darray_at(DArray *da, size_t offset, void *value)
{
    if (offset < da->length) {
        memcpy(value, OFFSET_TO_ADDR(da, offset), LENGTH(da, 1));
        return TRUE;
    } else {
        return FALSE;
    }
}

bool darray_shift(DArray *da, void *value)
{
    if (da->length > 0) {
        memcpy(value, OFFSET_TO_ADDR(da, 0), LENGTH(da, 1));
        if (--da->length > 0) {
            memmove(OFFSET_TO_ADDR(da, 0), OFFSET_TO_ADDR(da, 1), LENGTH(da, da->length));
        }
        return TRUE;
    } else {
        return FALSE;
    }
}

bool darray_pop(DArray *da, void *value)
{
    if (da->length > 0) {
        memcpy(value, OFFSET_TO_ADDR(da, --da->length), LENGTH(da, 1));
        return TRUE;
    } else {
        return FALSE;
    }
}

void darray_append_all(DArray *da, const void * const data, size_t data_length)
{
    darray_maybe_resize_of(da, data_length);
    memcpy(OFFSET_TO_ADDR(da, da->length), data, LENGTH(da, data_length));
    da->length += data_length;
}

void darray_prepend_all(DArray *da, const void * const data, size_t data_length)
{
    darray_maybe_resize_of(da, data_length);
    if (da->length > 0) {
        memmove(OFFSET_TO_ADDR(da, data_length), OFFSET_TO_ADDR(da, 0), LENGTH(da, da->length));
    }
    memcpy(OFFSET_TO_ADDR(da, 0), data, LENGTH(da, data_length));
    da->length += data_length;
}

void darray_insert_all(DArray *da, size_t offset, const void * const data, size_t data_length)
{
    assert(offset <= da->length);

    darray_maybe_resize_of(da, 1);
    if (offset != da->length) {
        memmove(OFFSET_TO_ADDR(da, offset + 1), OFFSET_TO_ADDR(da, offset), LENGTH(da, da->length - offset));
    }
    memcpy(OFFSET_TO_ADDR(da, offset), data, LENGTH(da, data_length));
    da->length += data_length;
}

bool darray_remove_at(DArray *da, size_t offset)
{
    if (offset < da->length) {
        darray_maybe_dtor(da, offset, offset);
        memmove(OFFSET_TO_ADDR(da, offset), OFFSET_TO_ADDR(da, offset + 1), LENGTH(da, da->length - offset - 1));
        da->length--;
        return TRUE;
    } else {
        return FALSE;
    }
}

void darray_remove_range(DArray *da, size_t from, size_t to)
{
    size_t diff;

    assert(from < da->length);
    assert(to < da->length);
    assert(from <= to);

    diff = ++to - from/* + 1*/;
    darray_maybe_dtor(da, from, to);
    memmove(OFFSET_TO_ADDR(da, from), OFFSET_TO_ADDR(da, to/* + 1*/), LENGTH(da, da->length - diff));
    da->length -= diff;
}

void darray_swap(DArray *da, size_t offset1, size_t offset2)
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

void darray_set_size(DArray *da, size_t length)
{
    if (length < da->length) {
        darray_maybe_dtor(da, length - 1, da->length);
        da->length = length;
    } else {
        darray_maybe_resize_to(da, length);
    }
}

size_t darray_length(DArray *da)
{
    return da->length;
}

void darray_sort(DArray *da, CmpFuncArg cmpfn, void *arg)
{
    assert(NULL != da);
    assert(NULL != cmpfn);

    qsort_r(da->data, da->length, da->element_size,
#ifdef BSD
        arg, cmpfn
#else
        cmpfn, arg
#endif
    );
}
