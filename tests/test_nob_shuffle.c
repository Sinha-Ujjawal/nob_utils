#include <stdio.h>
#include <stdlib.h>

#define NOB_IMPLEMENTATION
#define NOB_EXT_IMPLEMENTATION
#define NOB_BISECT_IMPLEMENTATION
#define NOB_PRIME_IMPLEMENTATION
#define NOB_SHUFFLE_IMPLEMENTATION
#include "nob_utils.h"

int main(void) {
    srand(time(NULL));
    Shuffler shuffler = {0};
    size_t n = 100;
    size_t m = n << 1;
    shuffler_reset(&shuffler, n);
    printf("First %zu items from Shuffler(%zu):\n", m, n);
    for (size_t i = 0; i < m; i++) {
        size_t x = shuffler.x;
        size_t idx = shuffler_next_index(&shuffler);
        size_t a = shuffler.a;
        size_t b = shuffler.b;
        printf("i: %zu\ta: %zu\tb: %zu\tx: %zu\tindex: %zu\n", i, a, b, x, idx);
    }
}

