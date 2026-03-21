#ifndef NOB_PROFILER_H_
#define NOB_PROFILER_H_

/* Program profiler that uses RDTSC instruction available on x86-64 architecture
   for profiling a C/C++ program.
   Reference: https://www.computerenhance.com/p/profiling-recursive-blocks
*/

#include <stdint.h>

typedef uint32_t u32;
typedef uint64_t u64;
typedef double f64;

u64 nob_read_os_timer(void);
u64 nob_get_os_timer_freq(void);
u64 nob_read_cpu_timer(void);
f64 nob_guess_cpu_timer_freq(u32 wait_time_in_millis);
f64 nob_measure_time_in_millis_from_elapsed(u64 elapsed, f64 freq);

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

f64 nob_guess_cpu_timer_freq(u32 wait_time_in_millis) {
    u64 os_freq = nob_get_os_timer_freq();
    u64 os_wait = wait_time_in_millis * os_freq / 1000;
    u64 os_start = nob_read_os_timer();
    u64 os_elapsed = 0;
    u64 os_end = 0;
    u64 cpu_start = nob_read_cpu_timer();
    while (os_elapsed < os_wait) {
        os_end = nob_read_os_timer();
        os_elapsed = os_end - os_start;
    }
    u64 cpu_end = nob_read_cpu_timer();
    u64 cpu_elapsed = cpu_end - cpu_start;
    f64 wall_clock = (f64) os_elapsed / (f64) os_freq;
    return (f64) cpu_elapsed / wall_clock;
}

f64 nob_measure_time_in_millis_from_elapsed(u64 elapsed, f64 freq) {
    return ((f64) elapsed / freq) * 1000;
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
    #endif // NOB_UNSTRIP_PREFIX
#endif // NOB_PROFILER_STRIP_PREFIX_GUARD_

#endif // NOB_PROFILER_H_
