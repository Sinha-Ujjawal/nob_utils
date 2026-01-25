#define NOB_IMPLEMENTATION
#include "nob.h"

typedef struct {
    char **items;
    size_t count;
    size_t capacity;
} Test_Binary_Exec_Names;

Cmd cmd = {0};
Procs procs = {0};

bool build_test_binary_exec(Test_Binary_Exec_Names *test_binary_exec_names, const char *test_binary_exec, const char **test_source_files, size_t count) {
    NOB_ASSERT((count >= 1) && "We need at least one source file to build the binary");
    int rebuild_is_needed = needs_rebuild(test_binary_exec, test_source_files, count);
    if (rebuild_is_needed > 0) {
        cmd_append(&cmd, "cc", "-Wall", "-Wextra", "-o", test_binary_exec, test_source_files[0]);
        if (!cmd_run(&cmd, .async = &procs)) return NULL;
    }
    da_append(test_binary_exec_names, (char *) test_binary_exec);
    return true;
}

int main(int argc, char **argv) {
    GO_REBUILD_URSELF(argc, argv);
    int result = 1;

    Test_Binary_Exec_Names test_binary_exec_names = {0};

    if (!build_test_binary_exec(&test_binary_exec_names, "./test_nob_heapq", (const char **)(char* [2]){"test_nob_heapq.c", "nob_heapq.h"}, 2)) nob_return_defer(1);
    if (!build_test_binary_exec(&test_binary_exec_names, "./test_nob_deque", (const char **)(char* [2]){"test_nob_deque.c", "nob_deque.h"}, 2)) nob_return_defer(1);
    if (!nob_procs_wait_and_reset(&procs)) nob_return_defer(1);

    da_foreach(char *, name, &test_binary_exec_names) {
        nob_cmd_append(&cmd, *name);
        printf("--------------------------------------------------\n");
        nob_log(NOB_INFO, "Running test for %s", *name);
        if (!nob_cmd_run(&cmd)) nob_return_defer(1);
        printf("--------------------------------------------------\n");
    }

    result = 0;
defer:
    free(cmd.items);
    free(procs.items);
    free(test_binary_exec_names.items);
    return result;
}
