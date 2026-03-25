#ifndef NOB_PROFILER_H_
#define NOB_PROFILER_H_

/* Program profiler that uses RDTSC instruction available on x86-64 architecture
   for profiling a C/C++ program.
   Reference: https://www.computerenhance.com/p/profiling-recursive-blocks
*/

#include <stdint.h>
#ifndef NOB_PROFILER_NO_STDLIB
#include <stdlib.h>
#endif // NOB_PROFILER_NO_STDLIB

typedef uint32_t u32;
typedef uint64_t u64;
typedef double f64;

u64 nob_read_os_timer(void);
u64 nob_get_os_timer_freq(void);
u64 nob_read_cpu_timer(void);
f64 nob_guess_timer_freq(u32 wait_time_in_millis, u64 (*timer)(void));
#define nob_guess_cpu_timer_freq(wait_time_in_millis) nob_guess_timer_freq(wait_time_in_millis, nob_read_cpu_timer)
f64 nob_measure_time_in_millis_from_elapsed(u64 elapsed, f64 freq);

#ifndef NOB_PROFILER_ENABLED
#define NOB_PROFILER_ENABLED 0
#endif // NOB_PROFILER_ENABLED

#ifndef NOB_PROFILER_BLOCKS_ENABLED
#define NOB_PROFILER_BLOCKS_ENABLED 0
#endif // NOB_PROFILER_BLOCKS_ENABLED

#ifndef NOB_PROFILER_BLOCK_TIMER
#define NOB_PROFILER_BLOCK_TIMER nob_read_cpu_timer
#endif // NOB_PROFILER_BLOCK_TIMER

#ifndef NOB_PROFILER_BLOCK_TIMER_FREQ
#define NOB_PROFILER_BLOCK_TIMER_FREQ nob_guess_timer_freq(100, NOB_PROFILER_BLOCK_TIMER)
#endif // NOB_PROFILER_BLOCK_TIMER_FREQ

#ifndef NOB_PROFILER_NO_STDLIB
#define NOB_ANCHORS_RESERVE_SIZE (1 << 12)
#define NOB_BLOCKS_RESERVE_SIZE  (1 << 16)

typedef struct {
    size_t anchor_idx;
    const char *label;
    u64 total_elapsed_including_children;
    u64 total_elapsed_excluding_children;
    size_t hit_count;
    u64 first_start;
    size_t byte_count;
} Nob_Profile_Anchor;

typedef struct {
    Nob_Profile_Anchor *items;
    size_t count;
    size_t capacity;
} Nob_Profile_Anchors;

typedef struct {
    size_t anchor_idx;
    size_t parent_idx;
    u64 start;
    u64 old_total_elapsed_including_children;
} Nob_Profile_Block;

typedef struct {
    Nob_Profile_Block *items;
    size_t count;
    size_t capacity;
} Nob_Profile_Blocks;

typedef struct {
    Nob_Profile_Anchors anchors;
    Nob_Profile_Blocks blocks;
    u64 start;
    f64 timer_freq;
} Nob_Profiler;

void nob_reset_profiler(Nob_Profiler *profiler);
void nob_start_profile_at_anchor(Nob_Profiler *profiler, const char *label, size_t anchor_idx);
#define nob_start_profile(profiler, label) nob_start_profile_at_anchor(profiler, label, __COUNTER__ + 1);
void nob_end_profile(Nob_Profiler *profiler, size_t byte_count);
void nob_log_profiler(Nob_Profiler profiler);
#endif // NOB_PROFILER_NO_STDLIB

typedef enum {
    NOB_REPEATITION_TEST_MODE_UNINITIALIZED,
    NOB_REPEATITION_TEST_MODE_TESTING,
    NOB_REPEATITION_TEST_MODE_COMPLETED,
    NOB_REPEATITION_TEST_MODE_ERROR,
} Nob_Repeatition_Test_Mode;

typedef struct {
    u64 test_count;
    u64 total_time;
    u64 max_time;
    u64 min_time;
} Nob_Repeatition_Test_Result;

typedef struct {
    u64 target_processed_byte_count;
    u64 cpu_timer_freq;
    u64 try_for_time;
    u64 test_started_at;
    Nob_Repeatition_Test_Mode mode;
    u32 open_block_count;
    u32 closed_block_count;
    u64 time_accum_this_test;
    u64 bytes_accum_this_test;
    Nob_Repeatition_Test_Result result;
} Nob_Repeatition_Tester;

#define nob_repeatition_tester_error(tester, ...)       \
    do {                                                \
        tester->mode = NOB_REPEATITION_TEST_MODE_ERROR; \
        nob_log(ERROR, __VA_ARGS__);                    \
    } while(0)
void nob_repeatition_tester_new_test_wave(Nob_Repeatition_Tester *tester, u64 target_processed_byte_count, u64 timer_freq, u32 seconds_to_try);
void nob_repeatition_tester_begin_timer(Nob_Repeatition_Tester *tester);
void nob_repeatition_tester_end_timer(Nob_Repeatition_Tester *tester);
void nob_repeatition_tester_count_bytes(Nob_Repeatition_Tester *tester, size_t byte_count);
bool nob_repeatition_tester_is_testing(Nob_Repeatition_Tester *tester);

#ifdef NOB_PROFILER_IMPLEMENTATION

#if _WIN32

#include <intrin.h>
#include <windows.h>

u64 nob_read_os_timer(void)
{
    LARGE_INTEGER Value;
    QueryPerformanceCounter(&Value);
    return Value.QuadPart;
}

u64 nob_get_os_timer_freq(void)
{   // Number of ticks per second of the timer
    LARGE_INTEGER Freq;
    QueryPerformanceFrequency(&Freq);
    return Freq.QuadPart;
}

#else

#include <x86intrin.h>
#include <sys/time.h>

u64 nob_read_os_timer(void)
{
    struct timeval Value;
    gettimeofday(&Value, 0);

    u64 Result = nob_get_os_timer_freq()*(u64)Value.tv_sec + (u64)Value.tv_usec;
    return Result;
}

u64 nob_get_os_timer_freq(void)
{   // Number of ticks per second of the timer
    // On Posix based systems like Linux and MacOS, the os timer tick is basically in micro-secs.
    // 1 sec = 1000 * 1000 micro-secs
    return 1000000;
}

#endif

u64 nob_read_cpu_timer(void)
{
    // NOTE: If you were on ARM, you would need to replace __rdtsc
    // with one of their performance counter read instructions, depending
    // on which ones are available on your platform.
    return __rdtsc();
}

f64 nob_guess_timer_freq(u32 wait_time_in_millis, u64 (*timer)(void)) {
    u64 os_freq = nob_get_os_timer_freq();
    u64 os_wait = wait_time_in_millis * os_freq / 1000;
    u64 os_start = nob_read_os_timer();
    u64 os_elapsed = 0;
    u64 os_end = 0;
    u64 timer_start = timer();
    while (os_elapsed < os_wait) {
        os_end = nob_read_os_timer();
        os_elapsed = os_end - os_start;
    }
    u64 timer_end = timer();
    u64 timer_elapsed = timer_end - timer_start;
    f64 wall_clock = (f64) os_elapsed / (f64) os_freq;
    return (f64) timer_elapsed / wall_clock;
}

f64 nob_measure_time_in_millis_from_elapsed(u64 elapsed, f64 freq) {
    return ((f64) elapsed / freq) * 1000;
}

#ifndef NOB_PROFILER_NO_STDLIB

void nob_reset_profiler(Nob_Profiler *profiler) {
#if NOB_PROFILER_ENABLED
    profiler->anchors.count = 0;
    profiler->blocks.count = 0;
    nob_da_reserve(&profiler->anchors, NOB_ANCHORS_RESERVE_SIZE);
    nob_da_reserve(&profiler->blocks, NOB_BLOCKS_RESERVE_SIZE);
    nob_da_append(&profiler->anchors, ((Nob_Profile_Anchor) {0}));
    profiler->timer_freq = (f64) (NOB_PROFILER_BLOCK_TIMER_FREQ);
    profiler->start = NOB_PROFILER_BLOCK_TIMER();
#else
    UNUSED(profiler);
#endif // NOB_PROFILER_ENABLED
}

void nob_start_profile_at_anchor(Nob_Profiler *profiler, const char *label, size_t anchor_idx) {
#if NOB_PROFILER_ENABLED && NOB_PROFILER_BLOCKS_ENABLED
    while (anchor_idx >= profiler->anchors.count) {
        nob_da_append(&profiler->anchors, ((Nob_Profile_Anchor) {0}));
    }
    Nob_Profile_Anchor *anchor = &profiler->anchors.items[anchor_idx];
    anchor->anchor_idx = anchor_idx;
    anchor->label = label;
    Nob_Profile_Block block = {0};
    block.anchor_idx = anchor_idx;
    if (profiler->blocks.count > 0) {
        block.parent_idx = nob_da_last(&profiler->blocks).anchor_idx;
    }
    block.start = NOB_PROFILER_BLOCK_TIMER();
    if (anchor->first_start == 0) {
        anchor->first_start = block.start;
    }
    block.old_total_elapsed_including_children = anchor->total_elapsed_including_children;
    nob_da_append(&profiler->blocks, block);
#else
    UNUSED(profiler);
    UNUSED(label);
    UNUSED(anchor_idx);
#endif // NOB_PROFILER_ENABLED && NOB_PROFILER_BLOCKS_ENABLED
}

void nob_end_profile(Nob_Profiler *profiler, size_t byte_count) {
#if NOB_PROFILER_ENABLED && NOB_PROFILER_BLOCKS_ENABLED
    Nob_Profile_Block block = nob_da_pop(&profiler->blocks);
    u64 elapsed = NOB_PROFILER_BLOCK_TIMER() - block.start;
    Nob_Profile_Anchor *anchor = &profiler->anchors.items[block.anchor_idx];
    anchor->total_elapsed_excluding_children += elapsed;
    anchor->total_elapsed_including_children = block.old_total_elapsed_including_children + elapsed;
    anchor->hit_count++;
    if (profiler->blocks.count > 0) {
        Nob_Profile_Anchor *parent = &profiler->anchors.items[block.parent_idx];
        parent->total_elapsed_excluding_children -= elapsed;
    }
    anchor->byte_count += byte_count;
#else
    UNUSED(profiler);
    UNUSED(byte_count);
#endif // NOB_PROFILER_ENABLED && NOB_PROFILER_BLOCKS_ENABLED
}

int nob__cmp_by_first_start(const void *a, const void *b) {
    const Nob_Profile_Anchor *anchor_a = (const Nob_Profile_Anchor *)a;
    const Nob_Profile_Anchor *anchor_b = (const Nob_Profile_Anchor *)b;

    if (anchor_a->first_start < anchor_b->first_start) return -1;
    if (anchor_a->first_start > anchor_b->first_start) return 1;
    return 0;
}

int nob__cmp_by_anchor_idx(const void *a, const void *b) {
    const Nob_Profile_Anchor *anchor_a = (const Nob_Profile_Anchor *)a;
    const Nob_Profile_Anchor *anchor_b = (const Nob_Profile_Anchor *)b;

    if (anchor_a->anchor_idx < anchor_b->anchor_idx) return -1;
    if (anchor_a->anchor_idx > anchor_b->anchor_idx) return 1;
    return 0;
}

void nob_log_profiler(Nob_Profiler profiler) {
#if NOB_PROFILER_ENABLED
    assert(profiler.blocks.count == 0); // No open blocks should be present
    u64 total_elapsed = NOB_PROFILER_BLOCK_TIMER() - profiler.start;
    nob_log(INFO, "Total: %.2f ms (Timer Freq: %.2f)", nob_measure_time_in_millis_from_elapsed(total_elapsed, profiler.timer_freq), profiler.timer_freq);
#if NOB_PROFILER_BLOCKS_ENABLED
    qsort(profiler.anchors.items + 1, profiler.anchors.count - 1, sizeof(Nob_Profile_Anchor), nob__cmp_by_first_start);
    for (size_t i = 1; i < profiler.anchors.count; i++) {
        Nob_Profile_Anchor anchor = profiler.anchors.items[i];
        if (anchor.label == NULL) continue;
        f64 percent = 100 * (f64) anchor.total_elapsed_excluding_children / (f64) total_elapsed;
        f64 millis = nob_measure_time_in_millis_from_elapsed(anchor.total_elapsed_excluding_children, profiler.timer_freq);
        size_t mark = nob_temp_save();
        char *log_line = NULL;
        size_t measured_size = 0;
        bool is_measuring = true;
        for (size_t i = 0; i < 2; i++) {
            size_t log_line_ptr = 0;
            log_line_ptr += snprintf(is_measuring ? log_line : (log_line + log_line_ptr), is_measuring ? 0 : measured_size, "  %s[%lu]: %.2f ms (%.2f%%", anchor.label, anchor.hit_count, millis, percent);
            if (anchor.total_elapsed_excluding_children != anchor.total_elapsed_including_children) {
                f64 percent_with_children = 100 * (f64) anchor.total_elapsed_including_children / (f64) total_elapsed;
                log_line_ptr += snprintf(is_measuring ? log_line : (log_line + log_line_ptr), is_measuring ? 0 : measured_size, ", %.2f%% w/children", percent_with_children);
            }
            log_line_ptr += snprintf(is_measuring ? log_line : (log_line + log_line_ptr), is_measuring ? 0 : measured_size, ")");
            if (anchor.byte_count > 0) {
                const f64 MEGABYTES = 1024.0f * 1024.0f;
                const f64 GIGABYTES = MEGABYTES * 1024.0f;
                f64 seconds = (f64) anchor.total_elapsed_including_children / profiler.timer_freq;
                f64 bytespersecond = (f64) anchor.byte_count / seconds;
                f64 mbs = (f64) anchor.byte_count / MEGABYTES;
                f64 gbps = bytespersecond / GIGABYTES;
                log_line_ptr += snprintf(is_measuring ? log_line : (log_line + log_line_ptr), is_measuring ? 0 : measured_size, "  %.3fmb at %.2fgb/s", mbs, gbps);
            }
            if (log_line == NULL && is_measuring) {
                is_measuring = false;
                measured_size = log_line_ptr + 1;
                log_line = (char *) nob_temp_alloc(measured_size * sizeof(char));
            }
        }
        assert(log_line != NULL);
        nob_log(INFO, "%s", log_line);
        nob_temp_rewind(mark);
    }
    qsort(profiler.anchors.items + 1, profiler.anchors.count - 1, sizeof(Nob_Profile_Anchor), nob__cmp_by_anchor_idx);
#else
#endif // NOB_PROFILER_BLOCKS_ENABLED
#else
    UNUSED(profiler);
#endif // NOB_PROFILER_ENABLED
}

#endif // NOB_PROFILER_NO_STDLIB

void nob_repeatition_tester_new_test_wave(Nob_Repeatition_Tester *tester, u64 target_processed_byte_count, u64 cpu_timer_freq, u32 seconds_to_try) {
    if (tester->mode == NOB_REPEATITION_TEST_MODE_UNINITIALIZED) {
        tester->mode = NOB_REPEATITION_TEST_MODE_TESTING;
        tester->target_processed_byte_count = target_processed_byte_count;
        tester->cpu_timer_freq = cpu_timer_freq;
        tester->result.min_time = (u64)-1;
    } else if (tester->mode == NOB_REPEATITION_TEST_MODE_UNINITIALIZED) {
        tester->mode = NOB_REPEATITION_TEST_MODE_TESTING;
        if (tester->target_processed_byte_count != target_processed_byte_count) {
            nob_repeatition_tester_error(tester, "target_processed_byte_count changed. Previously %lu bytes, Attempted to change to %lu bytes", tester->target_processed_byte_count, target_processed_byte_count);
        }
        if (tester->cpu_timer_freq != cpu_timer_freq) {
            nob_repeatition_tester_error(tester, "cpu_timer_freq changed. Previously %lu, Attempted to change to %lu", tester->cpu_timer_freq, cpu_timer_freq);
        }
    }
    tester->try_for_time = seconds_to_try * cpu_timer_freq;
    tester->test_started_at = nob_read_cpu_timer();
}

void nob_repeatition_tester_begin_timer(Nob_Repeatition_Tester *tester) {
    tester->open_block_count++;
    tester->time_accum_this_test -= nob_read_cpu_timer();
}

void nob_repeatition_tester_end_timer(Nob_Repeatition_Tester *tester) {
    tester->closed_block_count++;
    tester->time_accum_this_test += nob_read_cpu_timer();
}

void nob_repeatition_tester_count_bytes(Nob_Repeatition_Tester *tester, size_t byte_count) {
    tester->bytes_accum_this_test += byte_count;
}

void nob__print_time(char const *label, f64 cpu_time, u64 cpu_timer_freq, u64 byte_count) {
    size_t mark = nob_temp_save();
    char *log_line = NULL;
    size_t measured_size = 0;
    bool is_measuring = true;
    for (size_t i = 0; i < 2; i++) {
        size_t log_line_ptr = 0;
        log_line_ptr += snprintf(is_measuring ? log_line : (log_line + log_line_ptr), is_measuring ? 0 : measured_size, "%s: %.0f", label, cpu_time);
        if(cpu_timer_freq)
        {
            f64 seconds = cpu_time / cpu_timer_freq;
            log_line_ptr += snprintf(is_measuring ? log_line : (log_line + log_line_ptr), is_measuring ? 0 : measured_size, " (%fms)", 1000.0f*seconds);

            if(byte_count)
            {
                const f64 MEGABYTES = 1024.0f * 1024.0f;
                const f64 GIGABYTES = MEGABYTES * 1024.0f;
                f64 best_bw = byte_count / (GIGABYTES * seconds);
                log_line_ptr += snprintf(is_measuring ? log_line : (log_line + log_line_ptr), is_measuring ? 0 : measured_size, " %fgb/s", best_bw);
            }
        }
        if (log_line == NULL && is_measuring) {
            is_measuring = false;
            measured_size = log_line_ptr + 1;
            log_line = (char *) nob_temp_alloc(measured_size * sizeof(char));
        }
    }
    assert(log_line != NULL);
    nob_log(INFO, "%s", log_line);
    nob_temp_rewind(mark);
}

void nob__print_result(Nob_Repeatition_Test_Result result, u64 cpu_timer_freq, u64 byte_count) {
    nob__print_time("Min", result.min_time, cpu_timer_freq, byte_count);
    nob__print_time("Max", result.max_time, cpu_timer_freq, byte_count);
    if(result.test_count) {
        nob__print_time("Avg", (f64) result.total_time / (f64) result.test_count,  cpu_timer_freq, byte_count);
    }
}

bool nob_repeatition_tester_is_testing(Nob_Repeatition_Tester *tester) {
    if (tester->mode == NOB_REPEATITION_TEST_MODE_TESTING) {
        u64 current_time = nob_read_cpu_timer();
        if (tester->open_block_count) {
            if (tester->open_block_count != tester->closed_block_count) {
                nob_repeatition_tester_error(tester, "Unbalanced begin/end time, open count: %u, end count: %u", tester->open_block_count, tester->closed_block_count);
            }
            if (tester->bytes_accum_this_test != tester->target_processed_byte_count) {
                nob_repeatition_tester_error(tester, "Processed byte count mismatch. Expected: %lu bytes, got %lu bytes", tester->target_processed_byte_count, tester->bytes_accum_this_test);
            }
            if (tester->mode == NOB_REPEATITION_TEST_MODE_TESTING) {
                Nob_Repeatition_Test_Result *result = &tester->result;
                u64 elapsed_time = tester->time_accum_this_test;
                result->test_count++;
                result->total_time += elapsed_time;
                if (result->max_time < elapsed_time) {
                    result->max_time = elapsed_time;
                }
                if (result->min_time > elapsed_time) {
                    result->min_time = elapsed_time;
                    tester->test_started_at = current_time;
                    nob__print_time("Min", result->min_time, tester->cpu_timer_freq, tester->target_processed_byte_count);
                }

                tester->open_block_count = 0;
                tester->closed_block_count = 0;
                tester->time_accum_this_test = 0;
                tester->bytes_accum_this_test = 0;
            }
        }
        if ((current_time - tester->test_started_at) > tester->try_for_time) {
            tester->mode = NOB_REPEATITION_TEST_MODE_COMPLETED;
            nob__print_result(tester->result, tester->cpu_timer_freq, tester->target_processed_byte_count);
        }
    }
    return tester->mode == NOB_REPEATITION_TEST_MODE_TESTING;
}

#endif // NOB_PROFILER_IMPLEMENTATION

#ifndef NOB_PROFILER_STRIP_PREFIX_GUARD_
#define NOB_PROFILER_STRIP_PREFIX_GUARD_
    #ifndef NOB_UNSTRIP_PREFIX
        #define read_os_timer                       nob_read_os_timer
        #define get_os_timer_freq                   nob_get_os_timer_freq
        #define read_cpu_timer                      nob_read_cpu_timer
        #define guess_cpu_timer_freq                nob_guess_cpu_timer_freq
        #define measure_time_in_millis_from_elapsed nob_measure_time_in_millis_from_elapsed
        #ifndef NOB_PROFILER_NO_STDLIB
            #define Profile_Anchor           Nob_Profile_Anchor
            #define Profile_Anchors          Nob_Profile_Anchors
            #define Profile_Block            Nob_Profile_Block
            #define Profile_Blocks           Nob_Profile_Blocks
            #define Profiler                 Nob_Profiler
            #define reset_profiler           nob_reset_profiler
            #define start_profile_at_anchor  nob_start_profile_at_anchor
            #define start_profile            nob_start_profile
            #define end_profile              nob_end_profile
            #define log_profiler             nob_log_profiler
        #endif // NOB_PROFILER_NO_STDLIB
        #define Repeatition_Test_Mode               Nob_Repeatition_Test_Mode
        #define REPEATITION_TEST_MODE_UNINITIALIZED NOB_REPEATITION_TEST_MODE_UNINITIALIZED
        #define REPEATITION_TEST_MODE_TESTING       NOB_REPEATITION_TEST_MODE_TESTING
        #define REPEATITION_TEST_MODE_COMPLETED     NOB_REPEATITION_TEST_MODE_COMPLETED
        #define REPEATITION_TEST_MODE_ERROR         NOB_REPEATITION_TEST_MODE_ERROR
        #define Repeatition_Test_Result             Nob_Repeatition_Test_Result
        #define Repeatition_Tester                  Nob_Repeatition_Tester
        #define repeatition_tester_error            nob_repeatition_tester_error
        #define repeatition_tester_new_test_wave    nob_repeatition_tester_new_test_wave
        #define repeatition_tester_begin_timer      nob_repeatition_tester_begin_timer
        #define repeatition_tester_end_timer        nob_repeatition_tester_end_timer
        #define repeatition_tester_count_bytes      nob_repeatition_tester_count_bytes
        #define repeatition_tester_is_testing       nob_repeatition_tester_is_testing
    #endif // NOB_UNSTRIP_PREFIX
#endif // NOB_PROFILER_STRIP_PREFIX_GUARD_

#endif // NOB_PROFILER_H_
