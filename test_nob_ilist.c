#include "nob.h"
#include "nob_utils.h"

typedef struct {
    size_t x;
    ILIST_FIELDS
} MyIlistNode;
MyIlistNode xs[10] = {0};

void printXs() {
    for (size_t i = 0; i < ARRAY_LEN(xs); i++) {
        MyIlistNode node = xs[i];
        printf("root: %zu\n", i);
        printf("  x: %zu\n", node.x);
        printf("  parent: %zu\n", node.parent);
        printf("  prevSibling: %zu\n", node.prevSibling);
        printf("  nextSibling: %zu\n", node.nextSibling);
        printf("  firstChild: %zu\n", node.firstChild);
        printf("  lastChild: %zu\n", node.lastChild);
    }
}

int main(void) {
    ilist_prepend(xs, 1, 2);
    printf("After prepending 2 to 1\n");
    printXs();

    ilist_prepend(xs, 1, 2);
    printf("After prepending 2 to 1 again.\n");
    printXs();

    ilist_prepend(xs, 1, 1);
    printf("After prepending 1 to itself.\n");
    printXs();

    ilist_append(xs, 1, 2);
    printf("After appending 2 to 1.\n");
    printXs();

    ilist_append(xs, 1, 3);
    printf("After appending 3 to 1.\n");
    printXs();

    ilist_append(xs, 2, 3);
    printf("After appending 3 to 2.\n");
    printXs();

    // NOTE: This creates a cycle. The current impl. does not check for this, in order to be fast
    // ilist_append(xs, 3, 2);
    // printf("After appending 2 to 3.\n");
    // printXs();

    ilist_shift(xs, 1);
    printf("After shifting 1\n");
    printXs();

    ilist_delink(xs, 2);
    printf("After delinking 2\n");
    printXs();
    return 0;
}

