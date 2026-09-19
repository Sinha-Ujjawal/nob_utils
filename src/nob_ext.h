#ifndef NOB_EXT_H_
#define NOB_EXT_H_

// Extensions of nob

#define nob_embed_da(T)  \
    struct {             \
        T *items;        \
        size_t count;    \
        size_t capacity; \
    }

#define nob_da_insert(da, idx, item)                                                                         \
    do {                                                                                                     \
        assert((idx) >= 0 && (idx) <= (da)->count);                                                          \
        nob_da_reserve((da), (da)->count + 1);                                                               \
        memmove((da)->items + (idx) + 1, (da)->items + (idx), ((da)->count - (idx)) * sizeof(*(da)->items)); \
        (da)->items[(idx)] = (item);                                                                         \
        (da)->count++;                                                                                       \
    } while(0)

#define nob_da_delete(da, idx)                                                                                   \
    do {                                                                                                         \
        assert((idx) >= 0 && (idx) < (da)->count);                                                               \
        memmove((da)->items + (idx), (da)->items + (idx) + 1, ((da)->count - (idx) - 1) * sizeof(*(da)->items)); \
        (da)->count--;                                                                                           \
    } while(0)

#define nob_da_prepend(da, item) nob_da_insert((da), 0, (item))

#define nob_rand_exclusive(min, max) \
    (assert((min) < (max)), (rand() % ((max) - (min))) + (min))

#define nob_rand_inclusive(min, max) \
    (assert((min) <= (max)), (rand() % ((max) - (min) + 1)) + (min))

bool nob_append_to_file(const char *path, const void *data, size_t size);

#endif // NOB_EXT_H_

#ifdef NOB_EXT_IMPLEMENTATION

// Stolen from nob_write_entire_file, with mode ab instead of wb
bool nob_append_to_file(const char *path, const void *data, size_t size) {
    bool result = true;

    const char *buf = NULL;
    FILE *f = fopen(path, "ab");
    if (f == NULL) {
        nob_log(NOB_ERROR, "Could not open file %s for writing in append mode: %s\n", path, strerror(errno));
        nob_return_defer(false);
    }

    //           len
    //           v
    // aaaaaaaaaa
    //     ^
    //     data

    buf = (const char*)data;
    while (size > 0) {
        size_t n = fwrite(buf, 1, size, f);
        if (ferror(f)) {
            nob_log(NOB_ERROR, "Could not write into file %s: %s\n", path, strerror(errno));
            nob_return_defer(false);
        }
        size -= n;
        buf  += n;
    }

defer:
    if (f) fclose(f);
    return result;
}
#endif // NOB_EXT_IMPLEMENTATION

#ifndef NOB_EXT_STRIP_PREFIX_GUARD_
#define NOB_EXT_STRIP_PREFIX_GUARD_
    #ifndef NOB_UNSTRIP_PREFIX
        #define embed_da       nob_embed_da
        #define da_insert      nob_da_insert
        #define da_delete      nob_da_delete
        #define da_prepend     nob_da_prepend
        #define rand_exclusive nob_rand_exclusive
        #define rand_inclusive nob_rand_inclusive
        #define append_to_file nob_append_to_file
    #endif // NOB_UNSTRIP_PREFIX
#endif // NOB_EXT_STRIP_PREFIX_GUARD_
