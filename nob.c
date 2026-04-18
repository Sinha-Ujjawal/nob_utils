#define NOB_IMPLEMENTATION
#include "thirdparty/nob.h"

#define BUILD_DIR "build"

Cmd cmd = {0};
Procs procs = {0};

typedef struct {
    char const* test_binary_exec;
    char const** source_files;
    size_t num_source_files;
    bool check_to_build_or_run;
} Test_Case;

#define mk_test(name, ...) {                                                                 \
    .test_binary_exec = BUILD_DIR "/" #name,                                                 \
    .source_files = (char const*[]){ "tests/" #name ".c", "thirdparty/nob.h", __VA_ARGS__ }, \
    .num_source_files = 2 + (sizeof((char const*[]){ __VA_ARGS__ }) / sizeof(char const*)),  \
}

Test_Case test_cases[] = {
    mk_test(test_nob_fa       , "src/nob_fa.h"),
    mk_test(test_nob_heapq    , "src/nob_heapq.h"),
    mk_test(test_nob_deque    , "src/nob_deque.h"),
    mk_test(test_nob_hash     , "src/nob_hash.h"),
    mk_test(test_nob_ht       , "src/nob_ht.h", "src/nob_hash.h"),
    mk_test(test_nob_ilist    , "src/nob_ilist.h"),
    mk_test(test_nob_profiler , "src/nob_profiler.h"),
    mk_test(test_nob_graph    , "src/nob_graph.h", "src/nob_deque.h", "src/nob_ht.h", "src/nob_hash.h"),
    mk_test(test_nob_rc       , "src/nob_rc.h"),
};

bool build(bool always_build) {
    for (size_t i = 0; i < ARRAY_LEN(test_cases); i++) {
        Test_Case test_case = test_cases[i];
        if (!test_case.check_to_build_or_run) continue;
        assert(test_case.num_source_files > 0);
        int rebuild_is_needed = 0;
        if (always_build) {
            rebuild_is_needed = 1;
        } else {
            rebuild_is_needed = needs_rebuild(test_case.test_binary_exec, test_case.source_files, test_case.num_source_files);
        }
        if (rebuild_is_needed > 0) {
            cmd_append(&cmd,
                "clang",
                "-I./thirdparty", "-I./src",
                "-O2",\
                "-Wall", "-Wextra", "-Werror", "-Wswitch-enum",
                "-o", test_case.test_binary_exec, test_case.source_files[0]);
            if (!cmd_run(&cmd, .async = &procs)) return NULL;
        }
    }
    if (!nob_procs_wait_and_reset(&procs)) return false;
    return true;
}

bool run() {
    for (size_t i = 0; i < ARRAY_LEN(test_cases); i++) {
        Test_Case test_case = test_cases[i];
        if (!test_case.check_to_build_or_run) continue;
        nob_cmd_append(&cmd, test_case.test_binary_exec);
        printf("--------------------------------------------------\n");
        nob_log(NOB_INFO, "Running test for %s", test_case.test_binary_exec);
        if (!nob_cmd_run(&cmd)) return false;
        printf("--------------------------------------------------\n");
    }
    return true;
}

int main(int argc, char **argv) {
    GO_REBUILD_URSELF(argc, argv);
    int result = 1;

    if (!mkdir_if_not_exists(BUILD_DIR)) return_defer(false);

    char const* program = shift(argv, argc);

    #define USAGE                                                                   \
        do {                                                                        \
            nob_log(INFO, "Usage: %s <sub-command> [-f] [test-cases...]", program); \
            nob_log(INFO, "SUBCOMMANDS:");                                          \
            nob_log(INFO, "  build: Only builds the test cases");                   \
            nob_log(INFO, "  run:   Builds and run the test cases");                \
            nob_log(INFO, "  help:  Prints this help message");                     \
            nob_log(INFO, "-f: Will forcefully build the test-files");              \
            nob_log(INFO, "test-cases: Lets you chose which test-cases to build");  \
            nob_log(INFO, "  Available test-cases");                                \
            for (size_t i = 0; i < ARRAY_LEN(test_cases); i++) {                    \
                nob_log(INFO, "    %s", test_cases[i].test_binary_exec);            \
            }                                                                       \
        } while(0)

    if (argc <= 0) {
        USAGE;
        nob_log(ERROR, "No <sub-command> provided!");
        return_defer(1);
    }

    char const* subcommand = shift(argv, argc);
    bool always_build = false;
    bool any_test_cases_provided = false;

    while (argc > 0) {
        char const* opt = shift(argv, argc);
        if (strcmp(opt, "-f") == 0) {
            always_build = true;
            continue;
        }
        any_test_cases_provided = true;
        bool found = false;
        for (size_t i = 0; i < ARRAY_LEN(test_cases); i++) {
            if (strcmp(test_cases[i].test_binary_exec, opt) == 0) {
                found = true;
                test_cases[i].check_to_build_or_run = true;
                break;
            }
        }
        if (!found) {
            USAGE;
            nob_log(ERROR, "Invalid test-case: `%s` provided!", opt);
            return_defer(1);
        }
    }

    if (!any_test_cases_provided) {
        for (size_t i = 0; i < ARRAY_LEN(test_cases); i++) {
            test_cases[i].check_to_build_or_run = true;
        }
    }

    if (strcmp(subcommand, "build") == 0) {
        if (!build(always_build)) return_defer(1);
    } else if (strcmp(subcommand, "run") == 0) {
        if (!build(always_build)) return_defer(1);
        if (!run()) return_defer(1);
    } else if (strcmp(subcommand, "help") == 0) {
        USAGE;
        return_defer(0);
    } else {
        USAGE;
        nob_log(ERROR, "Unknown <sub-command>: `%s` provided!", subcommand);
        return_defer(1);
    }

    result = 0;
defer:
    free(cmd.items);
    free(procs.items);
    return result;
}
