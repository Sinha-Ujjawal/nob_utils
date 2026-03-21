#define NOB_PROFILER_IMPLEMENTATION
#include "nob_profiler.h"

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
    return 0;
}
