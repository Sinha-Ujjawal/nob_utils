#ifndef NOB_RC_H_
#define NOB_RC_H_

#include <stddef.h>

// A simple Reference Counting Pointer Impl. in C to automatically deallocate when the ref. count to a pointer reaches 0
// Reference: Tsoding Daily - [Reference Counting in C](https://youtu.be/iotrPxUnTdQ)

typedef struct {
    void (*destroy)(void *data);
#ifdef NOB_RC_TRACK_STATS
    size_t num_allocs;
    size_t num_acquires;
    size_t num_releases;
    size_t num_frees;
#endif // NOB_RC_TRACK_STATS
} Nob_RC_Allocator;

typedef struct {
#if defined NOB_RC_USE_8BITS
    uint8_t count;
#elif defined NOB_RC_USE_16BITS
    uint16_t count;
#elif defined NOB_RC_USE_64BITS
    uint64_t count;
#else
    uint32_t count; // 32-bits by default
#endif
} Nob_RC_Header;

// Allocates the memory of certain size along with the `Nob_RC_Header` header
void *nob_rc_alloc(Nob_RC_Allocator *allocator, size_t size);

// Acquires the pointer incrementing its ref. count
// NOTE: Assumes that the `ptr` was allocated using `nob_rc_alloc`.
void *nob_rc_acquire(Nob_RC_Allocator *allocator, void *ptr);

// Release the pointer decrementing its ref. count. When the ref. count reaches 0, automatically deallocates the memory
// NOTE: Assumes that the `ptr` was allocated using `nob_rc_alloc`.
#define nob_rc_release(allocator, ptr) nob__rc_release_impl((allocator), (void **) (&(ptr)))
void nob__rc_release_impl(Nob_RC_Allocator *allocator, void **ptr);

// Print Stats if tracked by the allocator
void nob_rc_print_stats(Nob_RC_Allocator allocator);

// Returns the ref. count of the pointer
size_t nob_rc_count(void *ptr);

#ifdef NOB_RC_IMPLEMENTATION

#include <assert.h>
#include <stdlib.h>

void *nob_rc_alloc(Nob_RC_Allocator *allocator, size_t size) {
    (void) allocator;
    assert(size > 0 && "Cannot allocate zero bytes :)!");
    size_t total_size = sizeof(Nob_RC_Header) + size;
    void *mem = malloc(total_size);
    assert(mem != NULL && "Buy more RAM lol");
    memset(mem, 0, total_size);
    void *ret = mem + sizeof(Nob_RC_Header);
#ifdef NOB_RC_LOG
    printf("RC: Allocating %zu bytes for Header (%lu) + Data(%zu); ptr: %p\n", total_size, sizeof(Nob_RC_Header), size, ret);
#endif // NOB_RC_LOG
#ifdef NOB_RC_TRACK_STATS
    allocator->num_allocs += 1;
#endif // NOB_RC_TRACK_STATS
    return ret;
}

void *nob_rc_acquire(Nob_RC_Allocator *allocator, void *ptr) {
    // NOTE: Assumes that the `ptr` was allocated using `nob_rc_alloc`.
    (void) allocator;
    assert(ptr != NULL);
    void *mem = ptr - sizeof(Nob_RC_Header);
    Nob_RC_Header *rc = (Nob_RC_Header *) mem;
    rc->count += 1;
#ifdef NOB_RC_LOG
    printf("RC: Acquired (count: %u) %p\n", rc->count, ptr);
#endif // NOB_RC_LOG
#ifdef NOB_RC_TRACK_STATS
    allocator->num_acquires += 1;
#endif // NOB_RC_TRACK_STATS
    return ptr;
}

void nob__rc_release_impl(Nob_RC_Allocator *allocator, void **ptr) {
    // NOTE: Assumes that the `ptr` was allocated using `nob_rc_alloc`.
#ifdef NOB_RC_TRACK_STATS
    allocator->num_releases += 1;
#endif // NOB_RC_TRACK_STATS
    assert(ptr != NULL);
    if (*ptr == NULL) {
#ifdef NOB_RC_LOG
        printf("RC: Trying to release a NULL pointer or already freed pointer :)!\n");
#endif
        return;
    }
    void *mem = *ptr - sizeof(Nob_RC_Header);
    Nob_RC_Header *rc = (Nob_RC_Header *) mem;
    if (rc->count > 0) {
        rc->count -= 1;
    }
#ifdef NOB_RC_LOG
    printf("RC: Released (count: %u) %p\n", rc->count, *ptr);
#endif // NOB_RC_LOG
    if (rc->count == 0) {
        allocator->destroy(*ptr);
        free(mem);
#ifdef NOB_RC_LOG
        printf("RC: Freed %p\n", *ptr);
#endif // NOB_RC_LOG
#ifdef NOB_RC_TRACK_STATS
        allocator->num_frees += 1;
#endif // NOB_RC_TRACK_STATS
        *ptr = NULL;
    }
}

void nob_rc_print_stats(Nob_RC_Allocator allocator) {
#ifdef NOB_RC_TRACK_STATS
    printf("RC: Total Allocations : %zu\n", allocator.num_allocs);
    printf("RC: Total Acquisitions: %zu\n", allocator.num_acquires);
    printf("RC: Total Releases    : %zu\n", allocator.num_releases);
    printf("RC: Total Frees       : %zu\n", allocator.num_frees);
#else
    (void) allocator;
#endif // NOB_RC_TRACK_STATS
}

size_t nob_rc_count(void *ptr) {
    void *mem = ptr - sizeof(Nob_RC_Header);
    Nob_RC_Header *rc = (Nob_RC_Header *) mem;
    return (size_t) rc->count;
}

#endif // NOB_RC_IMPLEMENTATION

#ifndef NOB_RC_STRIP_PREFIX_GUARD_
#define NOB_RC_STRIP_PREFIX_GUARD_
    #ifndef NOB_UNSTRIP_PREFIX
        #define RC_Allocator   Nob_RC_Allocator
        #define RC_Header      Nob_RC_Header
        #define rc_alloc       nob_rc_alloc
        #define rc_acquire     nob_rc_acquire
        #define rc_release     nob_rc_release
        #define rc_print_stats nob_rc_print_stats
        #define rc_count       nob_rc_count
    #endif // NOB_UNSTRIP_PREFIX
#endif // NOB_RC_STRIP_PREFIX_GUARD_

#endif // NOB_RC_H_
