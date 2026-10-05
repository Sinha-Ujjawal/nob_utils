#include "nob.h"
#define NOB_ILIST_IMPLEMENTATION
#include "nob_utils.h"

typedef struct {
    size_t x;
    embed_ilist_node;
} Nested;

typedef struct {
    Nested nestedField;
} MyIlistNode;
MyIlistNode xs[10] = {0};

void printXs() {
    for (size_t i = 0; i < ARRAY_LEN(xs); i++) {
        MyIlistNode node = xs[i];
        printf("root: %zu\n", i);
        printf("  x: %zu\n", node.nestedField.x);
        printf("  parent: %zu\n", node.nestedField.parent);
        printf("  prevSibling: %zu\n", node.nestedField.prevSibling);
        printf("  nextSibling: %zu\n", node.nestedField.nextSibling);
        printf("  firstChild: %zu\n", node.nestedField.firstChild);
    }
}

#define accessor(x) (x).nestedField

int main(void) {
    for (size_t i = 0; i < ARRAY_LEN(xs); i++) {
        xs[i].nestedField.x = i;
    }

    ilist_prepend(xs, accessor, 1, 2);
    printf("After prepending 2 to 1\n");
    printXs();

    ilist_prepend(xs, accessor, 1, 2);
    printf("After prepending 2 to 1 again.\n");
    printXs();

    ilist_prepend(xs, accessor, 1, 1);
    printf("After prepending 1 to itself.\n");
    printXs();

    ilist_append(xs, accessor, 1, 2);
    printf("After appending 2 to 1.\n");
    printXs();

    ilist_append(xs, accessor, 1, 3);
    printf("After appending 3 to 1.\n");
    printXs();
    printf("Foreach rooted at 1\n");
    ilist_foreach(Nested, it, &xs, accessor, 1) {
        printf("  x: %zu\n", it->x);
        printf("  parent: %zu\n", it->parent);
        printf("  prevSibling: %zu\n", it->prevSibling);
        printf("  nextSibling: %zu\n", it->nextSibling);
        printf("  firstChild: %zu\n", it->firstChild);
    }

    ilist_append(xs, accessor, 2, 3);
    printf("After appending 3 to 2.\n");
    printXs();

    // NOTE: This creates a cycle. The current impl. does not check for this, in order to be fast
    // ilist_append(xs, accessor, 3, 2);
    // printf("After appending 2 to 3.\n");
    // printXs();

    ilist_shift(xs, accessor, 1);
    printf("After shifting 1\n");
    printXs();

    ilist_delink(xs, accessor, 2);
    printf("After delinking 2\n");
    printXs();
    return 0;
}

