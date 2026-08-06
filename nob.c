#include <assert.h>

#define NOB_IMPLEMENTATION
#include "thirdparty/nob.h"

#define BUILD "build"
#define BUILD_REPORT_PATH "./build_report.md"

Cmd cmd = {0};
Procs procs = {0};

typedef enum {
    LINUX,
    MACOS,
    WINDOWS,
    __count_OS,
} OS;
const char *OS_NAMES[] = {
    "linux-gnu",
    "macos-none",
    "windows-gnu",
};
static_assert(ARRAY_LEN(OS_NAMES) == __count_OS, "Implement for missing OS!");

typedef enum {
   X86_64, 
   ARM64,
   __count_ARCH,
} ARCH;
const char *ARCH_NAMES[] = {
    "x86_64",
    "aarch64",
};
static_assert(ARRAY_LEN(ARCH_NAMES) == __count_ARCH, "Implement for missing ARCH!");

typedef struct {
    char const* exec_name;
    char const** source_files;
    size_t num_source_files;
    bool build;
    bool run;
    bool needs_explicit_run;
} Test_Case;

#define mk_test(name, explicit, ...) {                                                       \
    .exec_name = #name,                                                                      \
    .source_files = (char const*[]){ "tests/" #name ".c", "thirdparty/nob.h", __VA_ARGS__ }, \
    .num_source_files = 2 + (sizeof((char const*[]){ __VA_ARGS__ }) / sizeof(char const*)),  \
    .needs_explicit_run  = (explicit),                                                       \
}

Test_Case test_cases[] = {
    mk_test(test_nob_fa                        , false, "src/nob_fa.h"),
    mk_test(test_nob_heapq                     , false, "src/nob_heapq.h"),
    mk_test(test_nob_deque                     , false, "src/nob_deque.h"),
    mk_test(test_nob_fixed_deque               , false, "src/nob_fixed_deque.h"),
    mk_test(test_nob_hash                      , false, "src/nob_hash.h"),
    mk_test(test_nob_ht                        , false, "src/nob_ht.h", "src/nob_hash.h"),
    mk_test(test_nob_ilist                     , false, "src/nob_ilist.h"),
    mk_test(test_nob_entity                    , false, "src/nob_entity.h", "src/nob_ilist.h"),
    mk_test(test_nob_graph                     , false, "src/nob_graph.h", "src/nob_deque.h", "src/nob_ht.h", "src/nob_hash.h"),
    mk_test(test_nob_rc                        , false, "src/nob_rc.h"),
    mk_test(test_nob_br                        , false, "src/nob_br.h"),
    mk_test(test_nob_jsonrpc                   , false, "src/nob_jsonrpc.h", "thirdparty/jim.h", "thirdparty/jimp.h"),
    mk_test(test_nob_mcp                       , false, "src/nob_mcp.h", "src/nob_jsonrpc.h", "thirdparty/jim.h", "thirdparty/jimp.h"),
    mk_test(test_nob_channels                  , false, "src/nob_channels.h", "src/nob_deque.h", "src/nob_fixed_deque.h"),
    mk_test(test_nob_profiler                  , true , "src/nob_profiler.h"),
    mk_test(test_nob_profile_da_vs_deque       , true , "src/nob_profiler.h", "src/nob_fa.h", "src/nob_deque.h"),
    mk_test(test_nob_profile_alloc_huge_page   , true , "src/nob_profiler.h", "src/nob_fa.h", "src/nob_huge_page_alloc.h"),
    mk_test(test_nob_profile_fp_div_vs_fp_mul  , true , "src/nob_profiler.h"),
    mk_test(test_nob_profile_int_div_vs_int_mul, true , "src/nob_profiler.h"),
    mk_test(test_nob_bisect                    , false, "src/nob_bisect.h"),
    mk_test(test_nob_prime                     , false, "src/nob_bisect.h", "src/nob_prime.h"),
    mk_test(test_nob_shuffle                   , false, "src/nob_bisect.h", "src/nob_prime.h", "src/nob_shuffle.h"),
};

typedef struct {
    const char *test_case_name;
    ARCH arch;
    OS os;
} Test_Case_Build_Result;

#define get_target(arch, os) temp_sprintf("%s-%s", ARCH_NAMES[arch], OS_NAMES[os]);
#define get_target_dir(arch, os) temp_sprintf(BUILD"/%s-%s", ARCH_NAMES[arch], OS_NAMES[os]);

bool build(bool always_build) {
    struct {
        Test_Case_Build_Result *items;
        size_t count;
        size_t capacity;
    } test_case_build_results = {0};
    String_Builder sb = {0};
    bool result = false;
#define get_build_paths(test_case_name, arch, os, target, output_path, stdout_path, stderr_path) \
    do {                                                                          \
        target = temp_sprintf("%s-%s", ARCH_NAMES[arch], OS_NAMES[os]);           \
        output_path = temp_sprintf(BUILD"/%s/%s", target, test_case_name);        \
        stdout_path = temp_sprintf(BUILD"/%s/%s.stdout", target, test_case_name); \
        stderr_path = temp_sprintf(BUILD"/%s/%s.stderr", target, test_case_name); \
    } while(0)
    for (size_t i = 0; i < ARRAY_LEN(test_cases); i++) {
        Test_Case test_case = test_cases[i];
        if (!test_case.build) continue;
        assert(test_case.num_source_files > 0);
        int rebuild_is_needed = 0;
        if (always_build) {
            rebuild_is_needed = 1;
        } else {
            rebuild_is_needed = needs_rebuild(test_case.exec_name, test_case.source_files, test_case.num_source_files);
        }
        if (rebuild_is_needed > 0) {
            for (size_t arch = 0; arch < __count_ARCH; arch++) {
                for (size_t os = 0; os < __count_OS; os++) {
                    size_t saved = temp_save();
                    const char *target = NULL;
                    const char *output_path = NULL;
                    const char *stdout_path = NULL;
                    const char *stderr_path = NULL;
                    get_build_paths(test_case.exec_name, arch, os, target, output_path, stdout_path, stderr_path);
                    cmd_append(&cmd,
                        "zig", "cc",
                        "-target", target,
                        "-I./thirdparty", "-I./src",
                        "-O1",\
                        "-Wall", "-Wextra", "-Werror", "-Wswitch-enum",
                        "-Wno-unused-variable", "-Wno-unused-but-set-variable", "-Wno-format", "-Wno-error=gcc-install-dir-libstdcxx", // TODO: fix these warning errors
                        "-ggdb",
                        "-o", output_path,
                        test_case.source_files[0]
                    );
                    cmd_run(&cmd, .async = &procs, .max_procs = ARRAY_LEN(test_cases) * __count_OS * __count_ARCH, .stdout_path=stdout_path, .stderr_path=stderr_path);
                    temp_rewind(saved);
                    da_append(&test_case_build_results, ((Test_Case_Build_Result) {
                        .test_case_name = test_case.exec_name,
                        .arch = arch,
                        .os = os,
                    }));
                }
            }
        }
    }
    procs_flush(&procs);
    result = true;
    sb.count = 0;
    da_foreach(Test_Case_Build_Result, it, &test_case_build_results) {
        size_t saved = temp_save();
        const char *target = NULL;
        const char *output_path = NULL;
        const char *stdout_path = NULL;
        const char *stderr_path = NULL;
        get_build_paths(it->test_case_name, it->arch, it->os, target, output_path, stdout_path, stderr_path);
        sb_appendf(&sb, "```\nTest Case: %s\nTarget: %s\nErrors:\n", it->test_case_name, target);
        size_t count_before = sb.count;
        if (!read_entire_file(stderr_path, &sb)) return_defer(false);
        bool test_result = sb.count == count_before;
        result = test_result && result;
        temp_rewind(saved);
        sb_appendf(&sb, "\nStatus: %s\n```\n", test_result ? "✅ Success" : "❌ Failure");
    }
    if (!write_entire_file(BUILD_REPORT_PATH, sb.items, sb.count)) return_defer(false);
defer:
    free(test_case_build_results.items);
    free(sb.items);
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
    for (size_t arch = 0; arch < __count_ARCH; arch++) {
        for (size_t os = 0; os < __count_OS; os++) {
            size_t saved = temp_save();
            const char *target_dir = get_target_dir(arch, os);
            if (!mkdir_if_not_exists(target_dir)) return_defer(1);
            temp_rewind(saved);
        }
    }

    char const* program = shift(argv, argc);

    #define USAGE                                                                                                                \
        do {                                                                                                                     \
            nob_log(INFO, "Usage: %s <sub-command> [-f] [test-cases...]", program);                                              \
            nob_log(INFO, "SUBCOMMANDS:");                                                                                       \
            nob_log(INFO, "  build: Only builds the test cases");                                                                \
            nob_log(INFO, "  run:   Builds and run the test cases");                                                             \
            nob_log(INFO, "  help:  Prints this help message");                                                                  \
            nob_log(INFO, "-f: Will forcefully build the test-files");                                                           \
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
