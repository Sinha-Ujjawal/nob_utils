#ifndef NOB_EXT_H_
#define NOB_EXT_H_

// Extensions of nob

#define nob_embed_da(T)  \
    struct {             \
        T *items;        \
        size_t count;    \
        size_t capacity; \
    }

#define nob_da_prepend(da, item)                                                   \
    do {                                                                           \
        nob_da_reserve((da), (da)->count + 1);                                     \
        memmove((da)->items + 1, (da)->items, (da)->count * sizeof(*(da)->items)); \
        (da)->items[0] = (item);                                                   \
        (da)->count++;                                                             \
    } while(0)

#define nob_rand_exclusive(min, max) \
    (assert((min) < (max)), (rand() % ((max) - (min))) + (min))

#define nob_rand_inclusive(min, max) \
    (assert((min) <= (max)), (rand() % ((max) - (min) + 1)) + (min))

#endif // NOB_EXT_H_

#ifndef NOB_EXT_STRIP_PREFIX_GUARD_
#define NOB_EXT_STRIP_PREFIX_GUARD_
    #ifndef NOB_UNSTRIP_PREFIX
        #define embed_da       nob_embed_da
        #define da_prepend     nob_da_prepend
        #define rand_exclusive nob_rand_exclusive
        #define rand_inclusive nob_rand_inclusive
    #endif // NOB_UNSTRIP_PREFIX
#endif // NOB_EXT_STRIP_PREFIX_GUARD_
