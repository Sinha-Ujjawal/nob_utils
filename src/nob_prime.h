#ifndef NOB_PRIME_H_
#define NOB_PRIME_H_

// A simple nob library for checking if a number is prime using
// Sieve of Eratosthenes: https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes
// Binary Search: https://en.wikipedia.org/wiki/Binary_search

// Dependencies-
// 1. nob.h
// 2. nob_ext.h
// 3. nob_bisect.h

// The library maintains a global dynamic array of prime numbers
// Incrementally inserts into this dynamic array for new numbers greater than the max

typedef struct {
    nob_embed_da(size_t);
} Nob__Primes;

bool nob_is_prime(size_t x);
size_t nob_next_prime_gt(size_t x);
size_t nob_next_prime_ge(size_t x);

#endif // NOB_PRIME_H_

#ifdef NOB_PRIME_IMPLEMENTATION

Nob__Primes nob__primes = {0};

#if _WIN32
int nob__lte_for_size_t(void *arg, const void *p1, const void *p2) {
#else
int nob__lte_for_size_t(const void *p1, const void *p2, void *arg) {
#endif
    NOB_UNUSED(arg);
    size_t v1 = *((size_t *) p1);
    size_t v2 = *((size_t *) p2);
    return v1 - v2;
}

void nob__ensure_prime_array_is_filled(size_t x) {
    if (nob__primes.count == 0) {
        nob_da_append(&nob__primes, 2);
        nob_da_append(&nob__primes, 3);
    }

    if (x > nob_da_last(&nob__primes)) {
        size_t i = nob_da_last(&nob__primes);
        while (i <= x) {
            bool is_i_prime = true;
            nob_da_foreach(size_t, it, &nob__primes) {
                if (*it * *it > i) {
                    break;
                }
                if (i % *it == 0) {
                    is_i_prime = false;
                    break;
                }
            }
            if (is_i_prime) {
                nob_da_append(&nob__primes, i);
            }
            i += 2;
        }
    }
}

bool nob_is_prime(size_t x) {
    nob__ensure_prime_array_is_filled(x);
    int idx = nob_bisect_index(
        nob__primes.items, nob__primes.count, sizeof(*nob__primes.items),
        &x, nob__lte_for_size_t, NULL);
    return idx >= 0;
}

size_t nob_next_prime_gt(size_t x) {
    if (x < 2) return 2;
    if (x == 2) return 3;
    size_t y = x;
    if ((x & 1) == 0) y += 1; // increment y if x is even and not 2
    const size_t *ptr = NULL;
    while (ptr == NULL) {
        nob__ensure_prime_array_is_filled(y);
        ptr = nob_bisect_find_gt(
            nob__primes.items, nob__primes.count, sizeof(*nob__primes.items),
            &x, nob__lte_for_size_t, NULL);
        if (ptr == NULL) y += 2;
    }
    return *ptr;
}

size_t nob_next_prime_ge(size_t x) {
    if (x <= 2) return 2;
    if ((x & 1) == 0) x += 1; // increment if x is even and not 2
    size_t y = x;
    if ((x & 1) == 0) y += 1; // increment y if x is even and not 2
    const size_t *ptr = NULL;
    while (ptr == NULL) {
        nob__ensure_prime_array_is_filled(y);
        ptr = nob_bisect_find_ge(
            nob__primes.items, nob__primes.count, sizeof(*nob__primes.items),
            &x, nob__lte_for_size_t, NULL);
        if (ptr == NULL) y += 2;
    }
    return *ptr;
}

#endif // NOB_PRIME_IMPLEMENTATION

#ifndef NOB_PRIME_STRIP_PREFIX_GUARD_
#define NOB_PRIME_STRIP_PREFIX_GUARD_
    #ifndef NOB_UNSTRIP_PREFIX
        #define is_prime      nob_is_prime
        #define next_prime_gt nob_next_prime_gt
        #define next_prime_ge nob_next_prime_ge
    #endif // NOB_UNSTRIP_PREFIX
#endif // NOB_PRIME_STRIP_PREFIX_GUARD_
