#include <stdio.h>

#include "nob.h"
#include "nob_utils.h"

typedef struct {
    int *items;
    size_t begin;
    size_t count;
    size_t capacity;
} Ints;

int main(void) {
    Ints ints_dq = {0};

    for (int i = 0; i < 20; i++) {
        deque_append(&ints_dq, i);
        deque_prepend(&ints_dq, i * i);
    }

    printf("Printing all items using deque_foreach\n");
    deque_foreach(int, item, &ints_dq) {
        printf("%d\n", *item);
    }

    printf("Popping 7 elements from the end\n");
    for (int i = 0; i < 7; i++) {
        int item;
        deque_pop(&ints_dq, &item);
        printf("%d\n", item);
    }

    printf("Popping 5 elements from the beginning\n");
    for (int i = 0; i < 5; i++) {
        int item;
        deque_shift(&ints_dq, &item);
        printf("%d\n", item);
    }

    printf("Printing remaining items using deque_foreach\n");
    deque_foreach(int, item, &ints_dq) {
        printf("%d\n", *item);
    }

    free(ints_dq.items);
    return 0;
}
