#include <stdio.h>
#include <stdlib.h>

#define NOB_IMPLEMENTATION
#include "nob_heapq.h"

typedef struct {
    int *items;
    size_t count;
    size_t capacity;
} Heap;

Heap heap = {0};
const size_t heap_count = 10;
const int min_val = -10;
const int max_val = 10;
const int range = max_val - min_val + 1;
#define rand_int (rand() % range) + min_val

static inline void add_random_values(void) {
    static size_t random_seed = 69;
    srand(random_seed);
    heap.count = 0;
    printf("Emptying the heap and adding %zu random values (with seed: %zu)\n", heap_count, random_seed);
    for (size_t i = 0; i < heap_count; i++) {
        da_append(&heap, rand_int);
    }
}

static inline void reset_heap(void) {
    printf("Resetting heap\n");
    add_random_values();
    heapify(heap, 2, lt);
}

int main(int argc, char **argv) {
    GO_REBUILD_URSELF(argc, argv);

    add_random_values();

    printf("Before heapify\n");
    da_foreach(int, item, &heap) {
        printf("%d\n", *item);
    }

    heapify(heap, 2, lt);

    printf("After heapify\n");
    da_foreach(int, item, &heap) {
        printf("%d\n", *item);
    }

    heappush(&heap, 2, lt, 100);
    printf("After heappush 100\n");
    da_foreach(int, item, &heap) {
        printf("%d\n", *item);
    }

    printf("Sorted:\n");
    while (heap.count > 0) {
        int x;
        heappop(&heap, 2, lt, &x);
        printf("%d\n", x);
    }

    reset_heap();

    printf("Heappushpop:\n");
    for (size_t i = 0; i < 5; i++) {
        int x;
        int y = rand_int;
        int peek;
        heappeek(heap, &peek);
        heappushpop(heap, 2, lt, y, &x);
        printf("heappeak() -> %d, heappushpop(%d) -> %d\n", peek, y, x);
    }

    printf("Heapreplace:\n");
    for (size_t i = 0; i < 5; i++) {
        int x;
        int y = rand_int;
        int peek;
        heappeek(heap, &peek);
        heapreplace(heap, 2, lt, y, &x);
        printf("heappeak() -> %d, heapreplace(%d) -> %d\n", peek, y, x);
    }

    free(heap.items);
    return 0;
}
