#ifndef NOB_HEAPQ_H_
#define NOB_HEAPQ_H_

#define nob_lt(x, y) (x) <  (y)
#define nob_le(x, y) (x) <= (y)
#define nob_gt(x, y) (x) >  (y)
#define nob_ge(x, y) (x) >= (y)

#define nob_swap2(x, y) do {         \
    unsigned char _tmp[sizeof((x))]; \
    memcpy(_tmp, &(x), sizeof((x))); \
    memcpy(&(x), &(y), sizeof((x))); \
    memcpy(&(y), _tmp, sizeof((x))); \
} while (0)

#define nob__downheap(heap, arity, cmp_fn, start) do {                                       \
    NOB_ASSERT(arity >= 2 && "arity must at least be 2");                                    \
    size_t j = start;                                                                        \
    while (j < (heap).count) {                                                               \
        size_t root = j;                                                                     \
        for (size_t k = 1; k <= arity; k++) {                                                \
            size_t child = (j * arity) + k;                                                  \
            if ((child < (heap).count) && cmp_fn((heap).items[child], (heap).items[root])) { \
                root = child;                                                                \
            }                                                                                \
        }                                                                                    \
        if (root == j) {                                                                     \
            break;                                                                           \
        }                                                                                    \
        nob_swap2((heap).items[j], (heap).items[root]);                                      \
        j = root;                                                                            \
    }                                                                                        \
} while(0);

#define nob__upheap(heap, arity, cmp_fn, start) do {            \
    NOB_ASSERT(arity >= 2 && "arity must at least be 2");       \
    size_t j = start;                                           \
    while (j > 0) {                                             \
        size_t parent = (j - 1) / arity;                        \
        if (!(cmp_fn((heap).items[j], (heap).items[parent]))) { \
            break;                                              \
        }                                                       \
        nob_swap2((heap).items[j], (heap).items[parent]);       \
        j = parent;                                             \
    }                                                           \
} while(0);

#define nob_heapify(heap, arity, cmp_fn) do {             \
    NOB_ASSERT(arity >= 2 && "arity must at least be 2"); \
    for (size_t i = (heap).count / arity; i >= 1; i--) {  \
        nob__downheap(heap, arity, cmp_fn, (i - 1));      \
    }                                                     \
} while(0);

#define nob_heappeek(heap, ret) do {                                \
    NOB_ASSERT((heap).count > 0 && "Cannot peek into empty heap!"); \
    *ret = (heap).items[0];                                         \
} while(0);

#define nob_heappush(heap, arity, cmp_fn, entry) do {     \
    NOB_ASSERT(arity >= 2 && "arity must at least be 2"); \
    nob_da_append(heap, entry);                           \
    nob__upheap(*(heap), arity, cmp_fn, 0);               \
} while(0);

#define nob_heappop(heap, arity, cmp_fn, ret) do {                  \
    NOB_ASSERT(arity >= 2 && "arity must at least be 2");           \
    NOB_ASSERT((heap)->count > 0 && "Cannot pop from empty heap!"); \
    *(ret) = (heap)->items[0];                                      \
    nob_swap2((heap)->items[0], (heap)->items[(heap)->count - 1]);  \
    (heap)->count--;                                                \
    nob__downheap(*(heap), arity, cmp_fn, 0);                       \
} while(0);

#define nob_heappushpop(heap, arity, cmp_fn, entry, ret) do { \
    if ((heap).count == 0) {                                  \
        *(ret) = (entry);                                     \
        break;                                                \
    }                                                         \
    if (cmp_fn(entry, (heap).items[0])) {                     \
        *(ret) = (entry);                                     \
        break;                                                \
    }                                                         \
    *(ret) = (heap).items[0];                                 \
    (heap).items[0] = entry;                                  \
     nob__downheap(heap, arity, cmp_fn, 0);                   \
} while(0);

#define nob_heapreplace(heap, arity, cmp_fn, entry, ret) do { \
    if ((heap).count == 0) {                                  \
        *(ret) = (entry);                                     \
        break;                                                \
    }                                                         \
    *(ret) = (heap).items[0];                                 \
    (heap).items[0] = entry;                                  \
     nob__downheap(heap, arity, cmp_fn, 0);                   \
} while(0);
#define nob_heappoppush nob_heapreplace

#ifndef NOB_HEAPQ_STRIP_PREFIX_GUARD_
#define NOB_HEAPQ_STRIP_PREFIX_GUARD_
    #ifndef NOB_UNSTRIP_PREFIX
        #define lt           nob_lt
        #define le           nob_le
        #define gt           nob_gt
        #define ge           nob_ge
        #define swap2        nob_swap2
        #define heapify      nob_heapify
        #define heappeek     nob_heappeek
        #define heappush     nob_heappush
        #define heappop      nob_heappop
        #define heappushpop  nob_heappushpop
        #define heapreplace  nob_heapreplace
        #define heappoppush  nob_heappoppush
    #endif // NOB_UNSTRIP_PREFIX
#endif // NOB_HEAPQ_STRIP_PREFIX_GUARD_

#endif // NOB_HEAPQ_H_
