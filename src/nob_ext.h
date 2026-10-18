#ifndef NOB_EXT_H_
#define NOB_EXT_H_

// Extensions of nob

#define nob_embed_da(T)  \
    struct {             \
        T *items;        \
        size_t count;    \
        size_t capacity; \
    }

#define nob_da_insert(da, idx, item)                                                                         \
    do {                                                                                                     \
        assert((idx) >= 0 && (idx) <= (da)->count);                                                          \
        nob_da_reserve((da), (da)->count + 1);                                                               \
        memmove((da)->items + (idx) + 1, (da)->items + (idx), ((da)->count - (idx)) * sizeof(*(da)->items)); \
        (da)->items[(idx)] = (item);                                                                         \
        (da)->count++;                                                                                       \
    } while(0)

#define nob_da_delete(da, idx)                                                                                   \
    do {                                                                                                         \
        assert((idx) >= 0 && (idx) < (da)->count);                                                               \
        memmove((da)->items + (idx), (da)->items + (idx) + 1, ((da)->count - (idx) - 1) * sizeof(*(da)->items)); \
        (da)->count--;                                                                                           \
    } while(0)

#define nob_da_prepend(da, item) nob_da_insert((da), 0, (item))

double nob_lerp(double a, double b, double t);
float nob_lerpf(float a, float b, float t);

double nob_rand_inclusive(double min, double max);
float nob_rand_inclusivef(float min, float max);
int nob_rand_inclusivei(int min, int max);

double nob_clamp(double x, double min, double max);
float nob_clampf(float x, float min, float max);
bool nob_clampref(double *x, double min, double max);
bool nob_clampfref(float *x, float min, float max);

double nob_minmax_scaler(double x, double min, double max);
float nob_minmax_scalerf(float x, float min, float max);

bool nob_append_to_file(const char *path, const void *data, size_t size);

#endif // NOB_EXT_H_

#ifdef NOB_EXT_IMPLEMENTATION

double nob_lerp(double a, double b, double t) {
    NOB_ASSERT(a <= b);
    NOB_ASSERT(t >= 0);
    NOB_ASSERT(t <= 1);
    return a + t * (b - a);
}

float nob_lerpf(float a, float b, float t) {
    NOB_ASSERT(a <= b);
    NOB_ASSERT(t >= 0);
    NOB_ASSERT(t <= 1);
    return a + t * (b - a);
}

double nob_rand_inclusive(double min, double max) {
    NOB_ASSERT(min <= max);
    double t = (double) rand() / (double) INT_MAX;
    return nob_lerp(min, max, t);
}

float nob_rand_inclusivef(float min, float max) {
    NOB_ASSERT(min <= max);
    float t = (float) rand() / (float) INT_MAX;
    return nob_lerpf(min, max, t);
}

int nob_rand_inclusivei(int min, int max) {
    NOB_ASSERT(min <= max);
    return rand() % (max - min + 1) + min;
}

double nob_clamp(double x, double min, double max) {
    NOB_ASSERT(min <= max);
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

float nob_clampf(float x, float min, float max) {
    NOB_ASSERT(min <= max);
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

bool nob_clampref(double *x, double min, double max) {
    NOB_ASSERT(min <= max);
    if (*x < min) {
        *x = min;
        return true;
    }
    if (*x > max) {
        *x = max;
        return true;
    }
    return false;
}

bool nob_clampfref(float *x, float min, float max) {
    NOB_ASSERT(min <= max);
    if (*x < min) {
        *x = min;
        return true;
    }
    if (*x > max) {
        *x = max;
        return true;
    }
    return false;
}

double nob_minmax_scaler(double x, double min, double max) {
    NOB_ASSERT(min <= max);
    NOB_ASSERT(x >= min);
    NOB_ASSERT(x <= max);
    return (x - min) / (max - min);
}

float nob_minmax_scalerf(float x, float min, float max) {
    NOB_ASSERT(min <= max);
    NOB_ASSERT(x >= min);
    NOB_ASSERT(x <= max);
    return (x - min) / (max - min);
}

// Stolen from nob_write_entire_file, with mode ab instead of wb
bool nob_append_to_file(const char *path, const void *data, size_t size) {
    bool result = true;

    const char *buf = NULL;
    FILE *f = fopen(path, "ab");
    if (f == NULL) {
        nob_log(NOB_ERROR, "Could not open file %s for writing in append mode: %s\n", path, strerror(errno));
        nob_return_defer(false);
    }

    //           len
    //           v
    // aaaaaaaaaa
    //     ^
    //     data

    buf = (const char*)data;
    while (size > 0) {
        size_t n = fwrite(buf, 1, size, f);
        if (ferror(f)) {
            nob_log(NOB_ERROR, "Could not write into file %s: %s\n", path, strerror(errno));
            nob_return_defer(false);
        }
        size -= n;
        buf  += n;
    }

defer:
    if (f) fclose(f);
    return result;
}
#endif // NOB_EXT_IMPLEMENTATION

#ifndef NOB_EXT_STRIP_PREFIX_GUARD_
#define NOB_EXT_STRIP_PREFIX_GUARD_
    #ifndef NOB_UNSTRIP_PREFIX
        #define embed_da        nob_embed_da
        #define da_insert       nob_da_insert
        #define da_delete       nob_da_delete
        #define da_prepend      nob_da_prepend
        #define lerp            nob_lerp
        #define lerpf           nob_lerpf
        #define rand_inclusive  nob_rand_inclusive
        #define rand_inclusivef nob_rand_inclusivef
        #define rand_inclusivei nob_rand_inclusivei
        #define clamp           nob_clamp
        #define clampf          nob_clampf
        #define clampref        nob_clampref
        #define clampfref       nob_clampfref
        #define minmax_scaler   nob_minmax_scaler
        #define minmax_scalerf  nob_minmax_scalerf
        #define append_to_file  nob_append_to_file
    #endif // NOB_UNSTRIP_PREFIX
#endif // NOB_EXT_STRIP_PREFIX_GUARD_
