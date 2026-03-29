#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define NOB_IMPLEMENTATION
#include "nob.h"
#include "nob_utils.h"

// --- Heap Structures ---
typedef struct {
    int *items;
    size_t count;
    size_t capacity;
} Dynamic_Heap;

typedef struct {
    int items[200]; // Fixed capacity
    size_t count;
} Fixed_Heap;

// --- Test Constants ---
const size_t test_count = 128;
const int min_val = -10;
const int max_val = 10;
#define rand_int ((rand() % (max_val - min_val + 1)) + min_val)

// --- Generic Test Wrapper ---
// This macro runs the same test logic regardless of the underlying storage
#define RUN_HEAP_TEST(type_name, heap_ptr, append_macro, foreach_macro)                \
    do {                                                                               \
        printf("\n--- Testing %s ---\n", type_name);                                   \
        srand(69);                                                                     \
        (heap_ptr)->count = 0;                                                         \
                                                                                       \
        /* 1. Fill with random values */                                               \
        for (size_t i = 0; i < test_count; i++) {                                      \
            append_macro((heap_ptr), rand_int);                                        \
        }                                                                              \
                                                                                       \
        /* 2. Heapify */                                                               \
        heapify(*(heap_ptr), 2, lt);                                                   \
        printf("After heapify: ");                                                     \
        foreach_macro(int, item, (heap_ptr)) printf("%d ", *item);                     \
        printf("\n");                                                                  \
                                                                                       \
        /* 3. Push/Pop operations */                                                   \
        heappush((heap_ptr), 2, lt, -100);                                             \
        printf("After pushing -100 (min): %d\n", (heap_ptr)->items[0]);                \
                                                                                       \
        int popped;                                                                    \
        heappop((heap_ptr), 2, lt, &popped);                                           \
        printf("Popped min value: %d\n", popped);                                      \
                                                                                       \
        /* 4. PushPop / Replace */                                                     \
        int result;                                                                    \
        int val = 5;                                                                   \
        heappushpop(*(heap_ptr), 2, lt, val, &result);                                 \
        printf("heappushpop(%d) returned: %d\n", val, result);                         \
                                                                                       \
        /* 5. Drain and Verify Sort */                                                 \
        printf("Draining heap (sorted): ");                                            \
        while ((heap_ptr)->count > 0) {                                                \
            int x;                                                                     \
            heappop((heap_ptr), 2, lt, &x);                                            \
            printf("%d ", x);                                                          \
        }                                                                              \
        printf("\n");                                                                  \
    } while(0)

int main(void) {
    // Test Dynamic Array
    Dynamic_Heap d_heap = {0};
    RUN_HEAP_TEST("Dynamic Array", &d_heap, nob_da_append, nob_da_foreach);
    nob_da_free(d_heap);

    // Test Fixed Array
    Fixed_Heap f_heap = {0};
    #undef NOB_HEAP_APPEND
    #define NOB_HEAP_APPEND nob_fa_append
    RUN_HEAP_TEST("Fixed Array", &f_heap, nob_fa_append, nob_fa_foreach);

    return 0;
}
