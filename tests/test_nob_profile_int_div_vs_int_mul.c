#define NOB_IMPLEMENTATION
#define NOB_PROFILER_ENABLED 1
#define NOB_PROFILER_IMPLEMENTATION
#include "nob_utils.h"

int main(void) {
    Repeatition_Tester tester = {0};
    const u32 C32 = 3;
    const u64 C64 = 3;
    u64 cpu_timer_freq = guess_cpu_timer_freq(100);
    u64 seconds_to_try = 10;
#define repeatition_test_for_bits(BITS, n)                                       \
    do {                                                                         \
        u64 size = sizeof(u##BITS) * n;                                          \
        {                                                                        \
            nob_log(INFO, "u"#BITS"_div_%zu_KBs", size / (size_t) KILOBYTES(1)); \
            memset(&tester, 0, sizeof(Repeatition_Tester));                      \
            repeatition_test(                                                    \
                "u"#BITS"_div",                                                  \
                tester, cpu_timer_freq, seconds_to_try, size,                    \
                (                                                                \
                    u##BITS *mem = malloc(size);                                 \
                    for (u64 i = 0; i < n; i++) {                                \
                        mem[i] = (u##BITS) 1;                                    \
                    }                                                            \
                ),                                                               \
                (                                                                \
                    for (u64 i = 0; i < n; i++) {                                \
                        mem[i] /= C##BITS;                                       \
                    }                                                            \
                ),                                                               \
                (                                                                \
                    repeatition_tester_count_bytes(&tester, size);               \
                    free(mem);                                                   \
                )                                                                \
            );                                                                   \
        }                                                                        \
        {                                                                        \
            nob_log(INFO, "u"#BITS"_mul_%zu_KBs", size / (size_t) KILOBYTES(1)); \
            memset(&tester, 0, sizeof(Repeatition_Tester));                      \
            repeatition_test(                                                    \
                "u"#BITS"_mul",                                                  \
                tester, cpu_timer_freq, seconds_to_try, size,                    \
                (                                                                \
                    u##BITS *mem = malloc(size);                                 \
                    for (u64 i = 0; i < n; i++) {                                \
                        mem[i] = (u##BITS) 1;                                    \
                    }                                                            \
                ),                                                               \
                (                                                                \
                    for (u64 i = 0; i < n; i++) {                                \
                        mem[i] *= C##BITS;                                       \
                    }                                                            \
                ),                                                               \
                (                                                                \
                    repeatition_tester_count_bytes(&tester, size);               \
                    free(mem);                                                   \
                )                                                                \
            );                                                                   \
        }                                                                        \
    } while(0)
    for (u64 n = (1 << 10); n <= (1 << 30); n <<= 5) {
        repeatition_test_for_bits(32, n);
        repeatition_test_for_bits(64, n);
    }
    return 0;
}
