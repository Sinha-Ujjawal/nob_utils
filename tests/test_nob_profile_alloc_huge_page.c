#define NOB_IMPLEMENTATION
#define NOB_PROFILER_IMPLEMENTATION
#define NOB_PROFILER_ENABLED 1
#define NOB_PROFILER_BLOCKS_ENABLED 1
#define NOB_HUGE_PAGE_ALLOC_IMPLEMENTATION
#include "nob.h"
#include "nob_utils.h"

int main(void) {
    Repeatition_Tester tester = {0};
    u64 cpu_timer_freq = guess_cpu_timer_freq(100);
    u64 seconds_to_try = 10;
    for (u64 size = 4096; size <= 1024 * 1024 * 1024; size = size << 3) {
        {
            u64 mark = temp_save();
            memset(&tester, 0, sizeof(Repeatition_Tester));
            repeatition_test(
                temp_sprintf("Using_Malloc_%zu_KB", size / 1024),
                tester, cpu_timer_freq, seconds_to_try, size,
                (
                    u8 *arr = malloc(size);
                    assert(arr != NULL);
                ),
                (
                    for (u64 i = 0; i < size; ++i) arr[i] = (u8)i;
                    __asm__ volatile("" : : "g"(arr) : "memory"); // This avoids the compiler to optimize the array write
                ),
                (
                    repeatition_tester_count_bytes(&tester, size);
                    free(arr);
                )
            );
            temp_rewind(mark);
        }
        {
            u64 mark = temp_save();
            memset(&tester, 0, sizeof(Repeatition_Tester));
            bool log_huge_page_status_once = true;
            repeatition_test(
                temp_sprintf("Using_Huge_Page_Allocator_%zu_KB", size / 1024),
                tester, cpu_timer_freq, seconds_to_try, size,
                (
                    u8 *arr = NULL;
                    Huge_Page_Buffer hp_buff = {0};
                    if (try_alloc_huge_page(&hp_buff, size)) {
                        if (log_huge_page_status_once) {
                            log_huge_page_status_once = false;
                            nob_log(INFO, "Using Huge Page!");
                        }
                        arr = hp_buff.ptr;
                    } else {
                        if (log_huge_page_status_once) {
                            log_huge_page_status_once = false;
                            nob_log(INFO, "Cannot use Huge Page, fallback to Malloc");
                        }
                        arr = malloc(size);
                    }
                    assert(arr != NULL);
                ),
                (
                    for (u64 i = 0; i < size; ++i) arr[i] = (u8)i;
                    __asm__ volatile("" : : "g"(arr) : "memory"); // This avoids the compiler to optimize the array write
                ),
                (
                    repeatition_tester_count_bytes(&tester, size);
                    if (hp_buff.ptr != NULL) {
                        free_huge_page(&hp_buff);
                    } else {
                        free(arr);
                    }
                )
            );
            temp_rewind(mark);
        }
    }
    return 0;
}
