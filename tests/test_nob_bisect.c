#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include "nob.h"
#define NOB_BISECT_IMPLEMENTATION
#include "nob_bisect.h"

#if _WIN32
int is_lte_for_int(void *arg, const void *p1, const void *p2) {
#else
int is_lte_for_int(const void *p1, const void *p2, void *arg) {
#endif
    UNUSED(arg);
    int v1 = *((int *) p1);
    int v2 = *((int *) p2);
    return v1 - v2;
}

int main(void) {
    int arr[5] = {5, -1, 4, 1, 3};
#if _WIN32
    qsort_s(&arr, ARRAY_LEN(arr), sizeof(*arr), is_lte_for_int, NULL);
#else
    qsort_r(&arr, ARRAY_LEN(arr), sizeof(*arr), is_lte_for_int, NULL);
#endif
    printf("Sorted Items:\n");
    for (size_t i = 0; i < ARRAY_LEN(arr); i++) {
        printf("%d\n", arr[i]);
    }
#define find_index(x, bisect_algo)                                                                                 \
    do {                                                                                                           \
        size_t search_item = x;                                                                                    \
        int search_item_idx = bisect_algo(&arr, ARRAY_LEN(arr), sizeof(*arr), &search_item, is_lte_for_int, NULL); \
        printf(#bisect_algo"(%d): %d\n", search_item, search_item_idx);                                            \
    } while(0)
#define find_nearest_value(x, bisect_algo)                                                                               \
    do {                                                                                                                 \
        size_t search_item = x;                                                                                          \
        int *search_value = (int *) bisect_algo(&arr, ARRAY_LEN(arr), sizeof(*arr), &search_item, is_lte_for_int, NULL); \
        printf(#bisect_algo"(%d): ", search_item);                                                                       \
        if (search_value == NULL) {                                                                                      \
            printf("NOTFOUND\n");                                                                                        \
        } else {                                                                                                         \
            printf("%d\n", *search_value);                                                                               \
        }                                                                                                                \
    } while(0)
    
    printf("\nUsing bisect_left\n");
    find_index(2  , bisect_left);
    find_index(3  , bisect_left);
    find_index(4  , bisect_left);
    find_index(10 , bisect_left);
    find_index(-1 , bisect_left);
    find_index(-2 , bisect_left);

    printf("\nUsing bisect_right\n");
    find_index(2  , bisect_right);
    find_index(3  , bisect_right);
    find_index(4  , bisect_right);
    find_index(10 , bisect_right);
    find_index(-1 , bisect_right);
    find_index(-2 , bisect_right);

    printf("\nUsing bisect_index\n");
    find_index(2  , bisect_index);
    find_index(3  , bisect_index);
    find_index(4  , bisect_index);
    find_index(10 , bisect_index);
    find_index(-1 , bisect_index);
    find_index(-2 , bisect_index);

    printf("\nUsing bisect_find_lt\n");
    find_nearest_value(2  , bisect_find_lt);
    find_nearest_value(3  , bisect_find_lt);
    find_nearest_value(4  , bisect_find_lt);
    find_nearest_value(10 , bisect_find_lt);
    find_nearest_value(-1 , bisect_find_lt);
    find_nearest_value(-2 , bisect_find_lt);

    printf("\nUsing bisect_find_le\n");
    find_nearest_value(2  , bisect_find_le);
    find_nearest_value(3  , bisect_find_le);
    find_nearest_value(4  , bisect_find_le);
    find_nearest_value(10 , bisect_find_le);
    find_nearest_value(-1 , bisect_find_le);
    find_nearest_value(-2 , bisect_find_le);

    printf("\nUsing bisect_find_gt\n");
    find_nearest_value(2  , bisect_find_gt);
    find_nearest_value(3  , bisect_find_gt);
    find_nearest_value(4  , bisect_find_gt);
    find_nearest_value(10 , bisect_find_gt);
    find_nearest_value(-1 , bisect_find_gt);
    find_nearest_value(-2 , bisect_find_gt);

    printf("\nUsing bisect_find_ge\n");
    find_nearest_value(2  , bisect_find_ge);
    find_nearest_value(3  , bisect_find_ge);
    find_nearest_value(4  , bisect_find_ge);
    find_nearest_value(10 , bisect_find_ge);
    find_nearest_value(-1 , bisect_find_ge);
    find_nearest_value(-2 , bisect_find_ge);
    return 0;
}
