#include <stdio.h>

#define NOB_IMPLEMENTATION
#include "nob.h"
#include "nob_fa.h"

typedef struct {
    embed_fa(int, 10);
} Int_Stack;

int main(void) {
    Int_Stack stack = {0};

    // 1. Test fa_append
    nob_log(NOB_INFO, "Testing fa_append...");
    fa_append(&stack, 10);
    fa_append(&stack, 20);
    fa_append(&stack, 30);
    NOB_ASSERT(stack.count == 3);
    NOB_ASSERT(stack.items[0] == 10);
    NOB_ASSERT(stack.items[2] == 30);

    // 2. Test fa_first and fa_last
    nob_log(NOB_INFO, "Testing accessors...");
    NOB_ASSERT(fa_first(&stack) == 10);
    NOB_ASSERT(fa_last(&stack) == 30);

    // 3. Test fa_append_many
    nob_log(NOB_INFO, "Testing fa_append_many...");
    int more_items[] = {40, 50, 60};
    fa_append_many(&stack, more_items, 3);
    NOB_ASSERT(stack.count == 6);
    NOB_ASSERT(fa_last(&stack) == 60);

    // 4. Test fa_pop
    nob_log(NOB_INFO, "Testing fa_pop...");
    int popped = fa_pop(&stack);
    NOB_ASSERT(popped == 60);
    NOB_ASSERT(stack.count == 5);
    NOB_ASSERT(fa_last(&stack) == 50);

    // 5. Test fa_remove_unordered
    // Current stack: [10, 20, 30, 40, 50]
    nob_log(NOB_INFO, "Testing fa_remove_unordered...");
    fa_remove_unordered(&stack, 1); // Removes '20', swaps with '50'
    // New stack: [10, 50, 30, 40]
    NOB_ASSERT(stack.count == 4);
    NOB_ASSERT(stack.items[1] == 50);
    NOB_ASSERT(fa_last(&stack) == 40);

    // 6. Test fa_foreach
    nob_log(NOB_INFO, "Testing fa_foreach...");
    int sum = 0;
    int expected_sum = 10 + 50 + 30 + 40;
    fa_foreach(int, it, &stack) {
        sum += *it;
    }
    NOB_ASSERT(sum == expected_sum);

    printf("\n✅ All Fixed Array tests passed successfully!\n");

    return 0;
}
