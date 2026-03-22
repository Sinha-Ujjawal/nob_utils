#include <stdio.h>

#include "nob.h"
#define NOB_PROFILER_IMPLEMENTATION
#define NOB_PROFILER_ENABLED 1
#define NOB_PROFILER_BLOCKS_ENABLED 1
// #define NOB_PROFILER_NO_STDLIB
#include "nob_profiler.h"

Profiler profiler = {0};

u64 fibonacci_recursive(u64 n) {
    u64 result = 0;
    start_profile(&profiler, "fibonacci_recursive");

    if (n == 0) return_defer(0);
    if (n <= 2) return_defer(1);
    return_defer(fibonacci_recursive(n - 1) + fibonacci_recursive(n - 2));

defer:
    end_profile(&profiler);
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
    end_profile(&profiler);
    return f0;
}

u64 ackermann_recursive(u64 m, u64 n) {
    u64 result = 0;
    start_profile(&profiler, "ackermann_recursive");

    if (m == 0) {
        return_defer(n + 1);
    } else if (m > 0 && n == 0) {
        return_defer(ackermann_recursive(m - 1, 1));
    } else {
        return_defer(ackermann_recursive(m - 1, ackermann_recursive(m, n - 1)));
    }

defer:
    end_profile(&profiler);
    return result;
}

u64 tak(u64 x, u64 y, u64 z);

u64 tak_x(u64 x, u64 y, u64 z) {
    u64 result = 0;
    start_profile(&profiler, "tak_branch_x");
    return_defer(tak(x - 1, y, z));
defer:
    end_profile(&profiler);
    return result;
}

u64 tak_y(u64 x, u64 y, u64 z) {
    u64 result = 0;
    start_profile(&profiler, "tak_branch_y");
    return_defer(tak(y - 1, z, x));
defer:
    end_profile(&profiler);
    return result;
}

u64 tak_z(u64 x, u64 y, u64 z) {
    u64 result = 0;
    start_profile(&profiler, "tak_branch_z");
    return_defer(tak(z - 1, x, y));
defer:
    end_profile(&profiler);
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
    end_profile(&profiler);
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
        end_profile(&profiler);
        log_profiler(profiler);

        reset_profiler(&profiler);
        start_profile(&profiler, "fibonacci_iterative_start");
        printf("fibonacci_iterative(%lu): %lu\n", n, fibonacci_iterative(n));
        end_profile(&profiler);
        log_profiler(profiler);
    }

    {
       u64 n = 4, k = 1;
       reset_profiler(&profiler);
       start_profile(&profiler, "ackermann_recursive_start");
       printf("ackermann_recursive(%lu, %lu): %lu\n", n, k, ackermann_recursive(n, k));
       end_profile(&profiler);
       log_profiler(profiler);
    }

    {
       u64 x = 24, y = 16, z = 8;
       reset_profiler(&profiler);
       start_profile(&profiler, "tak_start");
       printf("tak(%lu, %lu, %lu): %lu\n", x, y, z, tak(x, y, z));
       end_profile(&profiler);
       log_profiler(profiler);
    }

    return 0;
}
