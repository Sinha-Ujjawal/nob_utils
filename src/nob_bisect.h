#ifndef NOB_BISECT_H_
#define NOB_BISECT_H_

// Implementing Array Bisection Algorithm
// References:
// - https://docs.python.org/3.13/library/bisect.html
// - https://github.com/python/cpython/blob/3.13/Lib/bisect.py

// The return value i is such that all e in a[:i] have e < x, and all e in
// a[i:] have e >= x.  So if x already appears in the list, a.insert(i, x) will
// insert just before the leftmost x already there.
size_t nob_bisect_left(
    const void *base, size_t nmemb, size_t size, const void *x,
    int (*compar)(const void *, const void *, void *),
    void *arg);

// The return value i is such that all e in a[:i] have e <= x, and all e in
// a[i:] have e > x.  So if x already appears in the list, a.insert(i, x) will
// insert just after the rightmost x already there.
size_t nob_bisect_right(
    const void *base, size_t nmemb, size_t size, const void *x,
    int (*compar)(const void *, const void *, void *),
    void *arg);

// Locate the leftmost value exactly equal to x
// If not found, then return -1
int nob_bisect_index(
    const void *base, size_t nmemb, size_t size, const void *x,
    int (*compar)(const void *, const void *, void *),
    void *arg);

// Find rightmost value less than x
// Returns the pointer to the element if found, otherwise returns NULL
const void * nob_bisect_find_lt(
    const void *base, size_t nmemb, size_t size, const void *x,
    int (*compar)(const void *, const void *, void *),
    void *arg);

// Find rightmost value less than or equal to x
// Returns the pointer to the element if found, otherwise returns NULL
const void * nob_bisect_find_le(
    const void *base, size_t nmemb, size_t size, const void *x,
    int (*compar)(const void *, const void *, void *),
    void *arg);

// Find leftmost value greater than x
// Returns the pointer to the element if found, otherwise returns NULL
const void * nob_bisect_find_gt(
    const void *base, size_t nmemb, size_t size, const void *x,
    int (*compar)(const void *, const void *, void *),
    void *arg);

// Find leftmost value greater than or equal to x
// Returns the pointer to the element if found, otherwise returns NULL
const void * nob_bisect_find_ge(
    const void *base, size_t nmemb, size_t size, const void *x,
    int (*compar)(const void *, const void *, void *),
    void *arg);

#endif // NOB_BISECT_H_

#ifdef NOB_BISECT_IMPLEMENTATION

// Index algorithms
size_t nob_bisect_left(
    const void *base, size_t nmemb, size_t size, const void *x,
    int (*compar)(const void *, const void *, void *),
    void *arg) {
    size_t lo = 0;
    size_t hi = nmemb;
    while (lo < hi) {
        size_t mid = lo + ((hi - lo) >> 1);
        if (compar(base + (mid*size), x, arg) < 0) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    return lo;
}

size_t nob_bisect_right(
    const void *base, size_t nmemb, size_t size, const void *x,
    int (*compar)(const void *, const void *, void *),
    void *arg) {
    size_t lo = 0;
    size_t hi = nmemb;
    while (lo < hi) {
        size_t mid = lo + ((hi - lo) >> 1);
        if (compar(x, base + (mid*size), arg) < 0) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }
    return lo;
}

int nob_bisect_index(
    const void *base, size_t nmemb, size_t size, const void *x,
    int (*compar)(const void *, const void *, void *),
    void *arg) {
    size_t i = nob_bisect_left(base, nmemb, size, x, compar, arg);
    if (i != nmemb && compar(x, base + (i*size), arg) == 0) {
        return i;
    }
    return -1;
}

// Nearest search algorithms
const void * nob_bisect_find_lt(
    const void *base, size_t nmemb, size_t size, const void *x,
    int (*compar)(const void *, const void *, void *),
    void *arg) {
    size_t i = nob_bisect_left(base, nmemb, size, x, compar, arg);
    if (i > 0) {
        return base + ((i-1)*size);
    }
    return NULL;
}

const void * nob_bisect_find_le(
    const void *base, size_t nmemb, size_t size, const void *x,
    int (*compar)(const void *, const void *, void *),
    void *arg) {
    size_t i = nob_bisect_right(base, nmemb, size, x, compar, arg);
    if (i > 0) {
        return base + ((i-1)*size);
    }
    return NULL;
}

const void * nob_bisect_find_gt(
    const void *base, size_t nmemb, size_t size, const void *x,
    int (*compar)(const void *, const void *, void *),
    void *arg) {
    size_t i = nob_bisect_right(base, nmemb, size, x, compar, arg);
    if (i != nmemb) {
        return base + (i*size);
    }
    return NULL;
}

const void * nob_bisect_find_ge(
    const void *base, size_t nmemb, size_t size, const void *x,
    int (*compar)(const void *, const void *, void *),
    void *arg) {
    size_t i = nob_bisect_left(base, nmemb, size, x, compar, arg);
    if (i != nmemb) {
        return base + (i*size);
    }
    return NULL;
}

#endif // NOB_BISECT_IMPLEMENTATION


#ifndef NOB_BISECT_STRIP_PREFIX_GUARD_
#define NOB_BISECT_STRIP_PREFIX_GUARD_
    #ifndef NOB_UNSTRIP_PREFIX
        // Index algorithms
        #define bisect_left  nob_bisect_left
        #define bisect_right nob_bisect_right
        #define bisect_index nob_bisect_index

        // Nearest search algorithms
        #define bisect_find_lt nob_bisect_find_lt
        #define bisect_find_le nob_bisect_find_le
        #define bisect_find_gt nob_bisect_find_gt
        #define bisect_find_ge nob_bisect_find_ge
    #endif // NOB_UNSTRIP_PREFIX
#endif // NOB_BISECT_STRIP_PREFIX_GUARD_
