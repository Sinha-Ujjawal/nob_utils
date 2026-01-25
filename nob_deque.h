#ifndef NOB_DEQUE_H_
#define NOB_DEQUE_H_

#include <stdlib.h>
#include <string.h>

// Initial capacity of a dynamic deque
#ifndef NOB_DEQUE_INIT_CAP
#define NOB_DEQUE_INIT_CAP 256
#endif

#define nob__deque_resize(dq, new_capacity)                                                        \
    do {                                                                                           \
        size_t new_begin = (new_capacity) >> 1;                                                    \
        void *new_items = malloc(sizeof(*(dq)->items) * (new_capacity));                           \
        NOB_ASSERT(new_items != NULL && "Buy more RAM lol");                                       \
        for (size_t i = 0; i < (dq)->count; i++) {                                                 \
            memcpy(                                                                                \
                (char*) new_items   + ( (new_begin  + i) % (new_capacity)) * sizeof(*(dq)->items), \
                (char*) (dq)->items + (((dq)->begin + i) % (dq)->capacity) * sizeof(*(dq)->items), \
                sizeof(*(dq)->items)                                                               \
            );                                                                                     \
        }                                                                                          \
        free((dq)->items);                                                                         \
        (dq)->capacity = (new_capacity);                                                           \
        (dq)->begin    = new_begin;                                                                \
        (dq)->items    = new_items;                                                                \
    } while(0)

#define nob_deque_reserve(dq, expected_capacity)         \
    do {                                                 \
        if ((expected_capacity) > (dq)->capacity) {      \
            size_t new_capacity = (dq)->capacity;        \
            if (new_capacity == 0) {                     \
                new_capacity = NOB_DEQUE_INIT_CAP;       \
            }                                            \
            while ((expected_capacity) > new_capacity) { \
                new_capacity *= 2;                       \
            }                                            \
            nob__deque_resize(dq, new_capacity);         \
        }                                                \
    } while (0)

// Append an item to a dynamic queue
#define nob_deque_append(dq, item)                                            \
    do {                                                                      \
        nob_deque_reserve((dq), (dq)->count + 1);                             \
        (dq)->items[((dq)->begin + (dq)->count) % ((dq)->capacity)] = (item); \
        (dq)->count++;                                                        \
    } while (0)

// Pop an item from the end in a dynamic queue
#define nob_deque_pop(dq, result)                                                           \
    do {                                                                                    \
        NOB_ASSERT(((dq)->count > 0) && "Cannot pop from empty deque!");                    \
        (dq)->count -= 1;                                                                   \
        *(result) = (dq)->items[((dq)->begin + (dq)->count) % (dq)->capacity];              \
        if (((dq)->count > NOB_DEQUE_INIT_CAP) && ((dq)->count <= (dq)->capacity * 0.25)) { \
            nob__deque_resize(dq, (dq)->capacity / 2);                                      \
        }                                                                                   \
    } while(0)

// Peek an item at the end in a dynamic queue
#define nob_deque_last(dq) \
    (dq)->items[NOB_ASSERT(((dq)->count > 0) && "Cannot peek into empty deque!"), ((dq)->begin + (dq)->count - 1) % (dq)->capacity];

// Prepends an item to a dynamic queue
#define nob_deque_prepend(dq, item)               \
    do {                                          \
        nob_deque_reserve((dq), (dq)->count + 1); \
        if ((dq)->begin == 0) {                   \
            (dq)->begin = (dq)->capacity;         \
        }                                         \
        (dq)->begin -= 1;                         \
        (dq)->items[(dq)->begin] = (item);        \
        (dq)->count++;                            \
    } while (0)

// Pop an item from the beginning in a dynamic queue
#define nob_deque_shift(dq, result)                                                         \
    do {                                                                                    \
        NOB_ASSERT(((dq)->count > 0) && "Cannot pop from empty deque!");                    \
        (dq)->count -= 1;                                                                   \
        *(result) = (dq)->items[(dq)->begin];                                               \
        (dq)->begin = ((dq)->begin + 1) % (dq)->capacity;                                   \
        if (((dq)->count > NOB_DEQUE_INIT_CAP) && ((dq)->count <= (dq)->capacity * 0.25)) { \
            nob__deque_resize(dq, (dq)->capacity / 2);                                      \
        }                                                                                   \
    } while(0)

// Peek an item at the beginning in a dynamic queue
#define nob_deque_first(dq) \
    (dq)->items[NOB_ASSERT(((dq)->count > 0) && "Cannot peek into empty deque!"), ((dq)->begin) % (dq)->capacity];


#define nob_deque_foreach(type, it, dq)       \
    for (size_t i = 0 ; i < (dq)->count; i++) \
        for (type *it = &(dq)->items[((dq)->begin + i) % (dq)->capacity]; it != NULL; it = NULL)

#ifndef NOB_DEQUE_STRIP_PREFIX_GUARD_
#define NOB_DEQUE_STRIP_PREFIX_GUARD_
    #ifndef NOB_UNSTRIP_PREFIX
        #define deque_reserve  nob_deque_reserve
        #define deque_append   nob_deque_append
        #define deque_pop      nob_deque_pop
        #define deque_last     nob_deque_last
        #define deque_prepend  nob_deque_prepend
        #define deque_shift    nob_deque_shift
        #define deque_first    nob_deque_first
        #define deque_foreach  nob_deque_foreach
    #endif // NOB_UNSTRIP_PREFIX
#endif // NOB_DEQUE_STRIP_PREFIX_GUARD_

#endif // NOB_DEQUE_H_
