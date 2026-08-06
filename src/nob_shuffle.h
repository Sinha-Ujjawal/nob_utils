#ifndef NOB_SHUFFLE_H_
#define NOB_SHUFFLE_H_

// Implemented random shuffler using (ax + b) % p to generate good non-repeating shuffles
// References:
// - Tsoding Daily (Coding Playlist Shuffle Algorithm): https://youtu.be/SWNqgv7w_cY

// There will be no repeatitions as proved below-
// Proof by contradiction-
//   Assume we had two different values x1 and x2 where f(x1) = f(x2), and 1 <= x1 < x2 < p
//   where, f(x) = (a*x + b) % p
//   f(x1) = f(x2)
//   subtracting f(x1) from both side
//   0 = f(x2) - f(x1)
//   ((a*x2 + b)%p - (a*x1 + b)%p) = 0
//   (a*x2 + b - a*x1 - b)%p = 0
//   (a*x2 - a*x1)%p = 0
//   a*(x2 - x1)%p = 0
//   This means that either a is divisible by p, or (x2 - x1) is divisible by p or both
//   But, that can never happen as a is a random number less than p
//   And, x2 - x1 is also a random number less than p
//   And, p is prime

// Dependencies:
// 1. nob.h
// 2. nob_ext.h
// 3. nob_bisect.h
// 4. nob_prime.h

typedef struct {
    size_t a;
    size_t b;
    size_t p;
    size_t x;
    size_t n;
} Nob_Shuffler;

void nob_shuffler_reset(Nob_Shuffler *shuffler, size_t n);
size_t nob_shuffler_next_index(Nob_Shuffler *shuffler);

#endif // NOB_SHUFFLE_H_

#ifdef NOB_SHUFFLE_IMPLEMENTATION

void nob_shuffler_reset(Nob_Shuffler *shuffler, size_t n) {
    memset(shuffler, 0, sizeof(Nob_Shuffler));
    shuffler->n = n;
    shuffler->p = nob_next_prime_ge(n);
}

size_t nob_shuffler_next_index(Nob_Shuffler *shuffler) {
    size_t ret;
    do {
        if (shuffler->x == 0) {
            shuffler->a = nob_rand_inclusive(1, shuffler->p - 1);
            shuffler->b = nob_rand_inclusive(1, shuffler->p - 1);
        }
        ret = (shuffler->a * shuffler->x + shuffler->b) % shuffler->p; // (ax + b) % p
        shuffler->x = (shuffler->x + 1) % (shuffler->p);
    } while(ret >= shuffler->n);
    return ret;
}

#endif // NOB_SHUFFLE_IMPLEMENTATION

#ifndef NOB_SHUFFLE_STRIP_PREFIX_GUARD_
#define NOB_SHUFFLE_STRIP_PREFIX_GUARD_
    #ifndef NOB_UNSTRIP_PREFIX
        #define Shuffler            Nob_Shuffler
        #define shuffler_reset      nob_shuffler_reset
        #define shuffler_next_index nob_shuffler_next_index
    #endif // NOB_UNSTRIP_PREFIX
#endif // NOB_SHUFFLE_STRIP_PREFIX_GUARD_
