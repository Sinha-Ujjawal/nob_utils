#include <stdio.h>

#define NOB_IMPLEMENTATION
#include "nob.h"
#define NOB_PROFILER_IMPLEMENTATION
#define NOB_PROFILER_ENABLED 1
#define NOB_PROFILER_BLOCKS_ENABLED 1
// #define NOB_PROFILER_BLOCK_TIMER nob_read_os_timer
// #define NOB_PROFILER_BLOCK_TIMER_FREQ nob_get_os_timer_freq()
// #define NOB_PROFILER_NO_STDLIB
#include "nob_profiler.h"

Profiler profiler = {0};

bool get_file_size(char const* file_path, size_t *out) {
    bool result = false;
    FILE *f = fopen(file_path, "rb");
    long long m = 0;
    if (f == NULL)                 return_defer(false);
    if (fseek(f, 0, SEEK_END) < 0) return_defer(false);
#ifndef _WIN32
    m = ftell(f);
#else
    m = _telli64(_fileno(f));
#endif
    if (m < 0)                     return_defer(false);
    if (fseek(f, 0, SEEK_SET) < 0) return_defer(false);
    if (out != NULL) {
        *out = m;
    }
    
    result = true;
defer:
    if (!result) nob_log(NOB_ERROR, "Could not read file %s: %s", file_path, strerror(errno));
    if (f) fclose(f);
    return result;
}

u64 fibonacci_recursive(u64 n) {
    u64 result = 0;
    start_profile(&profiler, "fibonacci_recursive");

    if (n == 0) return_defer(0);
    if (n <= 2) return_defer(1);
    return_defer(fibonacci_recursive(n - 1) + fibonacci_recursive(n - 2));

defer:
    end_profile(&profiler, 0);
    return result;
}

u64 fibonacci_iterative(u64 n) {
    start_profile(&profiler, "fibonacci_iterative");
    u64 f0 = 0;
    u64 f1 = 1;
    while (n > 0) {
        u64 temp = f0 + f1;
        f0 = f1;
        f1 = temp;
        n--;
    }
    end_profile(&profiler, 0);
    return f0;
}

u64 tak(u64 x, u64 y, u64 z);

u64 tak_x(u64 x, u64 y, u64 z) {
    u64 result = 0;
    start_profile(&profiler, "tak_branch_x");
    return_defer(tak(x - 1, y, z));
defer:
    end_profile(&profiler, 0);
    return result;
}

u64 tak_y(u64 x, u64 y, u64 z) {
    u64 result = 0;
    start_profile(&profiler, "tak_branch_y");
    return_defer(tak(y - 1, z, x));
defer:
    end_profile(&profiler, 0);
    return result;
}

u64 tak_z(u64 x, u64 y, u64 z) {
    u64 result = 0;
    start_profile(&profiler, "tak_branch_z");
    return_defer(tak(z - 1, x, y));
defer:
    end_profile(&profiler, 0);
    return result;
}

u64 tak(u64 x, u64 y, u64 z) {
    u64 result = 0;
    start_profile(&profiler, "tak_main");
    if (y < x) {
        return_defer(tak(tak_x(x, y, z), tak_y(x, y, z), tak_z(x, y, z)));
    } else {
        return_defer(z);
    }
defer:
    end_profile(&profiler, 0);
    return result;
}

int main(void) {
    f64 os_freq = get_os_timer_freq();
    f64 cpu_freq = guess_cpu_timer_freq(100);
    printf("OS Frequency: %f\n", os_freq);
    printf("Estimated CPU Frequency: %f\n", cpu_freq);
    u64 start_os = read_os_timer();
    u64 start_cpu = read_cpu_timer();
    size_t count = 1000000000;
    printf("Iterating over %zu integers\n", count);
    for (size_t i = 0; i < count; i++) {
        continue;
    }
    u64 elapsed_os = read_os_timer() - start_os;
    u64 elapsed_cpu = read_cpu_timer() - start_cpu;
    printf("OS Elapsed: %zu, Time Taken: %f millis\n", elapsed_os, measure_time_in_millis_from_elapsed(elapsed_os, os_freq));
    printf("CPU Elapsed: %zu, Time Taken: %f millis\n", elapsed_cpu, measure_time_in_millis_from_elapsed(elapsed_cpu, cpu_freq));

    {
        u64 n = 30;
        reset_profiler(&profiler);
        start_profile(&profiler, "fibonacci_recursive_start");
        printf("fibonacci_recursive(%lu): %lu\n", n, fibonacci_recursive(n));
        end_profile(&profiler, 0);
        log_profiler(profiler);

        reset_profiler(&profiler);
        start_profile(&profiler, "fibonacci_iterative_start");
        printf("fibonacci_iterative(%lu): %lu\n", n, fibonacci_iterative(n));
        end_profile(&profiler, 0);
        log_profiler(profiler);
    }

    {
       u64 x = 24, y = 16, z = 8;
       reset_profiler(&profiler);
       start_profile(&profiler, "tak_start");
       printf("tak(%lu, %lu, %lu): %lu\n", x, y, z, tak(x, y, z));
       end_profile(&profiler, 0);
       log_profiler(profiler);
    }

    {
        reset_profiler(&profiler);
        start_profile(&profiler, "malloc_1mb_test", .measure_page_faults=true);
        size_t size = 1024 * 1024; // 1mb;
        char *ptr = (char *) malloc(size);
        for (size_t i = 0; i < size; i++) {
            ptr[i] = ('a' + i) % 256;
        }
        end_profile(&profiler, size);
        log_profiler(profiler);
        nob_log(INFO, "ptr[0]: %c", ptr[0]);
        free(ptr);
    }

    {
        char const* file_path = __FILE__;
        char *buffer = NULL;

        size_t file_size;
        if (!get_file_size(file_path, &file_size)) return 1;
        buffer = realloc(buffer, file_size);
        Repeatition_Tester tester = {0};
        u64 cpu_timer_freq = (u64) guess_cpu_timer_freq(100);
        u64 seconds_to_try = 10;

        for (size_t i = 0; i < 3; i++) {
            repeatition_tester_new_test_wave(&tester, file_size, cpu_timer_freq, seconds_to_try);
            while (repeatition_tester_is_testing(&tester)) {
                repeatition_tester_begin_timer(&tester);
                    FILE *f = fopen(file_path, "rb");
                    if (f == NULL) {
                        nob_log(ERROR, "Could not open file: %s for reading: %s", file_path, strerror(errno));
                        return 1;
                    }
                    size_t bytes_read = fread(buffer, 1, file_size, f);
                    if (ferror(f)) {
                        nob_log(ERROR, "Could not read file %s: %s", file_path, strerror(errno));
                        return 1;
                    }
                    fclose(f);
                repeatition_tester_end_timer(&tester);
                repeatition_tester_count_bytes(&tester, bytes_read);
            }
        }
        free(buffer);
    }

    return 0;
}
