#include <assert.h>

#define NOB_IMPLEMENTATION
#include "thirdparty/nob.h"

#define NOB_EXT_IMPLEMENTATION
#include "src/nob_ext.h"

#define BUILD "build"
#define BUILD_REPORT_PATH "./build_report.md"

Cmd cmd = {0};
Procs procs = {0};

typedef enum {
    LINUX   = (1 << 0),
    MACOS   = (1 << 1),
    WINDOWS = (1 << 2),
} OS;
const char *OS_NAMES[] = {
    "linux-gnu",
    "macos-none",
    "windows-gnu",
};
static_assert(ARRAY_LEN(OS_NAMES) == 3, "Implement for missing OS!");
const size_t ALL_OS = LINUX | MACOS | WINDOWS;

typedef enum {
   X86_64 = (1 << 0),
   ARM64  = (1 << 1),
} ARCH;
const char *ARCH_NAMES[] = {
    "x86_64",
    "aarch64",
};
static_assert(ARRAY_LEN(ARCH_NAMES) == 2, "Implement for missing ARCH!");
const size_t ALL_ARCH = X86_64 | ARM64;

static_assert((ARRAY_LEN(OS_NAMES) * ARRAY_LEN(ARCH_NAMES)) <= sizeof(size_t), "size_t cannot hold the possible enumerations of os x architectures!");

typedef struct {
    char const* exec_name;
    char const** source_files;
    size_t num_source_files;
    bool build;
    bool run;
    bool needs_explicit_run;
    size_t supported_os;
    size_t supported_arch;
    struct {
        size_t done;
        size_t failed;
    } build_status;
} Test_Case;

#define mk_test(name, supported_os_, supported_arch_, explicit, ...) {                       \
    .exec_name = #name,                                                                      \
    .source_files = (char const*[]){ "tests/" #name ".c", "thirdparty/nob.h", __VA_ARGS__ }, \
    .num_source_files = 2 + (sizeof((char const*[]){ __VA_ARGS__ }) / sizeof(char const*)),  \
    .needs_explicit_run  = (explicit),                                                       \
    .supported_os = (supported_os_),                                                         \
    .supported_arch = (supported_arch_),                                                     \
}

Test_Case test_cases[] = {
    mk_test(test_nob_fa                        , ALL_OS       , ALL_ARCH, false, "src/nob_fa.h"),
    mk_test(test_nob_heapq                     , ALL_OS       , ALL_ARCH, false, "src/nob_heapq.h"),
    mk_test(test_nob_deque                     , ALL_OS       , ALL_ARCH, false, "src/nob_deque.h"),
    mk_test(test_nob_fixed_deque               , ALL_OS       , ALL_ARCH, false, "src/nob_fixed_deque.h"),
    mk_test(test_nob_hash                      , ALL_OS       , ALL_ARCH, false, "src/nob_hash.h"),
    mk_test(test_nob_ht                        , ALL_OS       , ALL_ARCH, false, "src/nob_ht.h", "src/nob_hash.h"),
    mk_test(test_nob_ilist                     , ALL_OS       , ALL_ARCH, false, "src/nob_ilist.h"),
    mk_test(test_nob_entity                    , ALL_OS       , ALL_ARCH, false, "src/nob_entity.h", "src/nob_ilist.h"),
    mk_test(test_nob_graph                     , ALL_OS       , ALL_ARCH, false, "src/nob_graph.h", "src/nob_deque.h", "src/nob_ht.h", "src/nob_hash.h"),
    mk_test(test_nob_rc                        , ALL_OS       , ALL_ARCH, false, "src/nob_rc.h"),
    mk_test(test_nob_br                        , LINUX | MACOS, ALL_ARCH, false, "src/nob_br.h"),
    mk_test(test_nob_jsonrpc                   , ALL_OS       , ALL_ARCH, false, "src/nob_jsonrpc.h", "thirdparty/jim.h", "thirdparty/jimp.h"),
    mk_test(test_nob_mcp                       , LINUX | MACOS, ALL_ARCH, false, "src/nob_mcp.h", "src/nob_jsonrpc.h", "thirdparty/jim.h", "thirdparty/jimp.h"),
    mk_test(test_nob_channels                  , ALL_OS       , ALL_ARCH, false, "src/nob_channels.h", "src/nob_deque.h", "src/nob_fixed_deque.h"),
    mk_test(test_nob_profiler                  , ALL_OS       , ALL_ARCH, true , "src/nob_profiler.h"),
    mk_test(test_nob_profile_da_vs_deque       , ALL_OS       , ALL_ARCH, true , "src/nob_profiler.h", "src/nob_fa.h", "src/nob_deque.h"),
    mk_test(test_nob_profile_alloc_huge_page   , ALL_OS       , ALL_ARCH, true , "src/nob_profiler.h", "src/nob_fa.h", "src/nob_huge_page_alloc.h"),
    mk_test(test_nob_profile_fp_div_vs_fp_mul  , ALL_OS       , ALL_ARCH, true , "src/nob_profiler.h"),
    mk_test(test_nob_profile_int_div_vs_int_mul, ALL_OS       , ALL_ARCH, true , "src/nob_profiler.h"),
    mk_test(test_nob_bisect                    , ALL_OS       , ALL_ARCH, false, "src/nob_bisect.h"),
    mk_test(test_nob_prime                     , ALL_OS       , ALL_ARCH, false, "src/nob_bisect.h", "src/nob_prime.h"),
    mk_test(test_nob_shuffle                   , ALL_OS       , ALL_ARCH, false, "src/nob_bisect.h", "src/nob_prime.h", "src/nob_shuffle.h"),
    mk_test(test_num_defs                      , ALL_OS       , ALL_ARCH, false, "src/num_defs.h"),
};

#define get_target(arch, os) temp_sprintf("%s-%s", ARCH_NAMES[arch], OS_NAMES[os]);
#define get_target_dir(arch, os) temp_sprintf(BUILD"/%s-%s", ARCH_NAMES[arch], OS_NAMES[os]);

bool build() {
    String_Builder test_results_summary = {0};
    String_Builder test_results_detail = {0};
    bool result = false;
#define get_build_paths(test_case_name, arch, os, target, output_path, stdout_path, stderr_path) \
    do {                                                                          \
        target = temp_sprintf("%s-%s", ARCH_NAMES[arch], OS_NAMES[os]);           \
        output_path = temp_sprintf(BUILD"/%s/%s", target, test_case_name);        \
        stdout_path = temp_sprintf(BUILD"/%s/%s.stdout", target, test_case_name); \
        stderr_path = temp_sprintf(BUILD"/%s/%s.stderr", target, test_case_name); \
    } while(0)
    for (size_t test_case_idx = 0; test_case_idx < ARRAY_LEN(test_cases); test_case_idx++) {
        Test_Case *test_case = &test_cases[test_case_idx];
        if (!test_case->build) continue;
        assert(test_case->num_source_files > 0);
        for (size_t arch = 0; arch < ARRAY_LEN(ARCH_NAMES); arch++) {
            if (!(test_case->supported_arch & (1 << arch))) {
                nob_log(INFO, "%s not supported for %s Architecture!", test_case->exec_name, ARCH_NAMES[arch]);
                continue;
            }
            for (size_t os = 0; os < ARRAY_LEN(OS_NAMES); os++) {
                if (!(test_case->supported_os & (1 << os))) {
                    nob_log(INFO, "%s not supported for %s OS!", test_case->exec_name, OS_NAMES[os]);
                    continue;
                }
                size_t build_status_idx = arch*ARRAY_LEN(OS_NAMES) + os;
                size_t saved = temp_save();
                const char *target = NULL;
                const char *output_path = NULL;
                const char *stdout_path = NULL;
                const char *stderr_path = NULL;
                get_build_paths(test_case->exec_name, arch, os, target, output_path, stdout_path, stderr_path);
                cmd_append(&cmd,
                    "zig", "cc",
                    "-target", target,
                    "-I./thirdparty", "-I./src",
                    "-O1",\
                    "-Wall", "-Wextra", "-Werror", "-Wswitch-enum",
                    "-Wno-unused-variable", "-Wno-unused-but-set-variable", "-Wno-format", // TODO: fix these warning errors
                    "-ggdb",
                    "-o", output_path,
                    test_case->source_files[0]
                );
                cmd_run(&cmd, .async = &procs, .max_procs = ARRAY_LEN(test_cases) * ARRAY_LEN(OS_NAMES) * ARRAY_LEN(ARCH_NAMES), .stdout_path=stdout_path, .stderr_path=stderr_path);
                temp_rewind(saved);
            }
        }
    }
    procs_flush(&procs);
    result = true;
    // Writing Test Results Detail
    test_results_detail.count = 0;
    for (size_t test_case_idx = 0; test_case_idx < ARRAY_LEN(test_cases); test_case_idx++) {
        Test_Case *test_case = &test_cases[test_case_idx];
        for (size_t arch = 0; arch < ARRAY_LEN(ARCH_NAMES); arch++) {
            if (!(test_case->supported_arch & (1 << arch))) {
                continue;
            }
            for (size_t os = 0; os < ARRAY_LEN(OS_NAMES); os++) {
                if (!(test_case->supported_os & (1 << os))) {
                    continue;
                }
                size_t build_status_idx = arch*ARRAY_LEN(OS_NAMES) + os;
                size_t saved = temp_save();
                const char *target = NULL;
                const char *output_path = NULL;
                const char *stdout_path = NULL;
                const char *stderr_path = NULL;
                get_build_paths(test_case->exec_name, arch, os, target, output_path, stdout_path, stderr_path);
                sb_appendf(&test_results_detail, "```\nTest Case: %s\nTarget: %s\nErrors:\n", test_case->exec_name, target);
                size_t count_before = test_results_detail.count;
                if (!read_entire_file(stderr_path, &test_results_detail)) return_defer(false);
                bool test_result = test_results_detail.count == count_before;
                result = test_result && result;
                temp_rewind(saved);
                sb_appendf(&test_results_detail, "\nStatus: %s\n```\n", test_result ? "✅ Success" : "❌ Failure");
                test_case->build_status.done |= 1 << build_status_idx;
                test_case->build_status.failed |= (!test_result) << build_status_idx;
            }
        }
    }
    // Writing Test Results Summary
    test_results_summary.count = 0;
    /// Header
    sb_append_cstr(&test_results_summary, "|test_case|");
    for (size_t arch = 0; arch < ARRAY_LEN(ARCH_NAMES); arch++) {
        for (size_t os = 0; os < ARRAY_LEN(OS_NAMES); os++) {
            sb_appendf(&test_results_summary, "%s-%s|", ARCH_NAMES[arch], OS_NAMES[os]);
        }
    }
    sb_append_cstr(&test_results_summary, "\n|---|");
    for (size_t arch = 0; arch < ARRAY_LEN(ARCH_NAMES); arch++) {
        for (size_t os = 0; os < ARRAY_LEN(OS_NAMES); os++) {
            sb_append_cstr(&test_results_summary, "---|");
        }
    }
    sb_append_cstr(&test_results_summary, "\n");
    for (size_t test_case_idx = 0; test_case_idx < ARRAY_LEN(test_cases); test_case_idx++) {
        Test_Case *test_case = &test_cases[test_case_idx];
        sb_appendf(&test_results_summary, "|%s|", test_case->exec_name);
        for (size_t arch = 0; arch < ARRAY_LEN(ARCH_NAMES); arch++) {
            bool arch_supported = true;
            if (!(test_case->supported_arch & (1 << arch))) {
                arch_supported = false;
            }
            for (size_t os = 0; os < ARRAY_LEN(OS_NAMES); os++) {
                bool os_supported = true;
                if (arch_supported && !(test_case->supported_os & (1 << os))) {
                    os_supported = false;
                }
                bool supported = arch_supported && os_supported;
                size_t build_status_idx = arch*ARRAY_LEN(OS_NAMES) + os;
                if (!supported) {
                    sb_append_cstr(&test_results_summary, "🚫 Not-Supported|");
                } else {
                    assert(test_case->build_status.done & (1 << build_status_idx));
                    bool test_result = !(test_case->build_status.failed & (1 << build_status_idx));
                    sb_append_cstr(&test_results_summary, test_result ? "✅ Success|" : "❌ Failure|");
                }
            }
        }
        sb_append_cstr(&test_results_summary, "\n");
    }
    sb_append_cstr(&test_results_summary, "\n");
    if (!write_entire_file(BUILD_REPORT_PATH, test_results_summary.items, test_results_summary.count)) return_defer(false);
    if (!append_to_file(BUILD_REPORT_PATH, test_results_detail.items, test_results_detail.count)) return_defer(false);
defer:
    free(test_results_summary.items);
    free(test_results_detail.items);
    return result;
}

bool run() {
    TODO("run");
    // for (size_t i = 0; i < ARRAY_LEN(test_cases); i++) {
    //     Test_Case test_case = test_cases[i];
    //     if (!test_case.run) continue;
    //     nob_cmd_append(&cmd, test_case.exec_name);
    //     printf("--------------------------------------------------\n");
    //     nob_log(NOB_INFO, "Running test for %s", test_case.exec_name);
    //     if (!nob_cmd_run(&cmd)) return false;
    //     printf("--------------------------------------------------\n");
    // }
    // return true;
}

int main(int argc, char **argv) {
    GO_REBUILD_URSELF(argc, argv);
    int result = 1;
    if (!mkdir_if_not_exists(BUILD)) return_defer(1);
    for (size_t arch = 0; arch < ARRAY_LEN(ARCH_NAMES); arch++) {
        for (size_t os = 0; os < ARRAY_LEN(OS_NAMES); os++) {
            size_t saved = temp_save();
            const char *target_dir = get_target_dir(arch, os);
            if (!mkdir_if_not_exists(target_dir)) return_defer(1);
            temp_rewind(saved);
        }
    }

    char const* program = shift(argv, argc);

    #define USAGE                                                                                                                \
        do {                                                                                                                     \
            nob_log(INFO, "Usage: %s <sub-command> [test-cases...]", program);                                              \
            nob_log(INFO, "SUBCOMMANDS:");                                                                                       \
            nob_log(INFO, "  build: Only builds the test cases");                                                                \
            nob_log(INFO, "  run:   Builds and run the test cases");                                                             \
            nob_log(INFO, "  help:  Prints this help message");                                                                  \
            nob_log(INFO, "test-cases: Lets you chose which test-cases to build");                                               \
            nob_log(INFO, "  Available test-cases");                                                                             \
            for (size_t i = 0; i < ARRAY_LEN(test_cases); i++) {                                                                 \
                nob_log(INFO, "    %s%s", test_cases[i].exec_name, test_cases[i].needs_explicit_run ? "[explicit]" : "");        \
            }                                                                                                                    \
            nob_log(INFO, "Note that the test-cases which are marked [explicit] needs to passed explcitly to run them");         \
        } while(0)

    if (argc <= 0) {
        USAGE;
        nob_log(ERROR, "No <sub-command> provided!");
        return_defer(1);
    }

    char const* subcommand = shift(argv, argc);
    bool any_test_cases_provided = false;

    while (argc > 0) {
        char const* opt = shift(argv, argc);
        any_test_cases_provided = true;
        bool found = false;
        for (size_t i = 0; i < ARRAY_LEN(test_cases); i++) {
            if (strcmp(test_cases[i].exec_name, opt) == 0) {
                found = true;
                test_cases[i].build = true;
                test_cases[i].run = true;
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
            test_cases[i].build = true;
            test_cases[i].run = !test_cases[i].needs_explicit_run && true;
        }
    }

    if (strcmp(subcommand, "build") == 0) {
        if (!build()) return_defer(1);
    } else if (strcmp(subcommand, "run") == 0) {
        if (!build()) return_defer(1);
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
