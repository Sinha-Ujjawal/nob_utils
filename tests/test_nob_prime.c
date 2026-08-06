#include <stdio.h>

#include "nob.h"
#include "nob_ext.h"
#define NOB_BISECT_IMPLEMENTATION
#include "nob_bisect.h"
#define NOB_PRIME_IMPLEMENTATION
#include "nob_prime.h"

int main(void) {
    size_t n = 1000000;
    printf("PRIMES till %zu:\n", n);
    for (size_t i = 1; i < n; i += 1) {
        if (is_prime(i)) {
            printf("%zu\n", i);
        }
    }
    for (size_t i = 1; i < 100; i++) {
        printf("Next Prime Greater >= %zu: %zu\n", i, next_prime_ge(i));
        printf("Next Prime Greater >  %zu: %zu\n", i, next_prime_gt(i));
    }
    return 0;
}
