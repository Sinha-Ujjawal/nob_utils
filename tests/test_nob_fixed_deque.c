#include <stdio.h>

#include "nob.h"
#include "nob_utils.h"

typedef struct {
    embed_fixed_deque(int, 100);
} Ints;

int main(void) {
    Ints ints_dq = {0};

    for (int i = 0; i < 20; i++) {
        fixed_deque_append(&ints_dq, i);
        fixed_deque_prepend(&ints_dq, i * i);
    }

    printf("Printing all items using fixed_deque_foreach\n");
    fixed_deque_foreach(int, item, &ints_dq) {
        printf("%d\n", *item);
    }

    printf("Popping 7 elements from the end\n");
    for (int i = 0; i < 7; i++) {
        int item;
        fixed_deque_pop(&ints_dq, &item);
        printf("%d\n", item);
    }

    printf("Popping 5 elements from the beginning\n");
    for (int i = 0; i < 5; i++) {
        int item;
        fixed_deque_shift(&ints_dq, &item);
        printf("%d\n", item);
    }

    printf("Printing remaining items using fixed_deque_foreach\n");
    fixed_deque_foreach(int, item, &ints_dq) {
        printf("%d\n", *item);
    }

    return 0;
}
