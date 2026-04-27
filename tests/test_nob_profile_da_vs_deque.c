#define NOB_IMPLEMENTATION
#define NOB_PROFILER_IMPLEMENTATION
#define NOB_PROFILER_ENABLED 1
#define NOB_PROFILER_BLOCKS_ENABLED 1
#define NOB_DA_INIT_CAP 256
#define NOB_DEQUE_INIT_CAP 256
#include "nob.h"
#include "nob_utils.h"

typedef struct {
    embed_da(u64);
} U64_DA;

typedef struct {
    embed_deque(u64);
} U64_DEQUE;

int main(void) {
    U64_DA da = {0};
    U64_DEQUE deq = {0};
    Repeatition_Tester tester = {0};
    u64 cpu_timer_freq = guess_cpu_timer_freq(100);
    u64 seconds_to_try = 10;
    for (size_t count = 1024; count < 1024 * 1024 * 1024; count = count << 3) {
        u64 total_size = count * sizeof(u64);

        {
            nob_log(INFO, "DA: Appending %zu KB", total_size / 1024);
            memset(&tester, 0, sizeof(Repeatition_Tester));
            repeatition_test(
                "Dynamic_Array",
                tester, cpu_timer_freq, seconds_to_try, total_size,
                (
                    da.count = 0;
                ),
                (
                    for (u64 i = 0; i < count; i++) {
                        da_append(&da, i);
                    }
                ),
                (
                    repeatition_tester_count_bytes(&tester, total_size);
                    da.count = 0;
                )
            );
        }

        {
            nob_log(INFO, "Deque: Appending %zu KB", total_size / 1024);
            memset(&tester, 0, sizeof(Repeatition_Tester));
            repeatition_test(
                "Dynamic_Deque",
                tester, cpu_timer_freq, seconds_to_try, total_size,
                (
                    deq.count = 0;
                ),
                (
                    for (u64 i = 0; i < count; i++) {
                        deque_append(&deq, i);
                    }
                ),
                (
                    repeatition_tester_count_bytes(&tester, total_size);
                    deq.count = 0;
                )
            );
        }
    }
    free(da.items);
    free(deq.items);
    return 0;
}
