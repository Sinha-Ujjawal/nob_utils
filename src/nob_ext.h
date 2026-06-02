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

#ifndef NOB_EXT_STRIP_PREFIX_GUARD_
#define NOB_EXT_STRIP_PREFIX_GUARD_
    #ifndef NOB_UNSTRIP_PREFIX
        #define embed_da   nob_embed_da
        #define da_prepend nob_da_prepend
    #endif // NOB_UNSTRIP_PREFIX
#endif // NOB_EXT_STRIP_PREFIX_GUARD_

#endif // NOB_EXT_H_
