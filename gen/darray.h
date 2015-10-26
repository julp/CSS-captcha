#ifndef DARRAY_H

# define DARRAY_H

# include <stdint.h>

typedef int bool;

#ifndef FALSE
# define FALSE 0
#endif /* !FALSE */
#ifndef TRUE
# define TRUE 1
#endif /* !TRUE */

typedef void (*DtorFunc)(void *);
typedef int (*CmpFuncArg)(const void *, const void *, void *);

typedef struct {
    uint8_t *data;
    size_t length;
    size_t allocated;
    size_t element_size;
    DtorFunc dtor_func;
} DArray;

# define darray_prepend(/*DArray **/ da, value) \
    darray_prepend((da), &(value), 1)

# define darray_push(/*DArray **/ da, value) \
    darray_append((da), (value))

# define darray_append(/*DArray **/ da, value) \
    darray_append_all((da), &(value), 1)

# define darray_insert(/*DArray **/ da, /*size_t*/ offset, value) \
    darray_insert((da), (offset), &(value), 1)

# define darray_at_unsafe(/*DArray **/ da, /*size_t*/ offset, T) \
    ((T *) ((void *) (da)->data))[(offset)]

# define darray_top_unsafe(/*DArray **/ da, T) \
    darray_at_unsafe(da, 0, T)

void darray_append_all(DArray *, const void * const, size_t);
bool darray_at(DArray *, size_t, void *);
void darray_clear(DArray *);
void darray_destroy(DArray *);
void darray_insert_all(DArray *, size_t, const void * const, size_t);
size_t darray_length(DArray *);
DArray *darray_new(DtorFunc, size_t);
bool darray_pop(DArray *, void *);
void darray_prepend_all(DArray *, const void * const, size_t);
bool darray_remove_at(DArray *, size_t);
void darray_remove_range(DArray *, size_t, size_t);
void darray_set_size(DArray *, size_t);
bool darray_shift(DArray *, void *);
DArray *darray_sized_new(DtorFunc, int32_t, size_t);
void darray_swap(DArray *, size_t, size_t);
void darray_sort(DArray *, CmpFuncArg, void *);

#endif /* !DARRAY_H */
