#define _GNU_SOURCE
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define NOB_IMPLEMENTATION
#define JIMP_IMPLEMENTATION
#define JIM_IMPLEMENTATION
#define NOB_JSONRPC_IMPLEMENTATION
#define NOB_BR_IMPLEMENTATION
#include "nob.h"
#include "jimp.h"
#include "jim.h"
#include "nob_utils.h"
#include "nob_jsonrpc.h"

// --- YOUR ORIGINAL TYPES AND LOGIC ---

typedef enum {
    METHOD_INIT,
    METHOD_SUBTRACT,
    METHOD_NOTIF,
} Method_Type;

typedef struct {
    Method_Type method_type;
    union {
        struct {
            String_View protocol_ver;
            String_View client_info_name;
            String_View client_info_ver;
        } init_params;
        struct {
            double x;
            double y;
        } subtract_params;
    };
} Params;

bool parse_params(void *ctx, String_View method, Jimp *jimp, void *ptr) {
    UNUSED(ctx);
    Params *params = ptr;
    if (sv_eq(method, sv_from_cstr("initialize"))) {
        params->method_type = METHOD_INIT;
        if (!jimp_object_begin(jimp)) return false;
        while (jimp_object_member(jimp)) {
            if (strcmp(jimp->string, "protocolVersion") == 0) {
                if (!jimp_string(jimp)) return false;
                params->init_params.protocol_ver = jimp_string_as_sv(jimp);
            } else if (strcmp(jimp->string, "clientInfo") == 0) {
                if (!jimp_object_begin(jimp)) return false;
                while (jimp_object_member(jimp)) {
                    if (strcmp(jimp->string, "name") == 0) {
                        if (!jimp_string(jimp)) return false;
                        params->init_params.client_info_name = jimp_string_as_sv(jimp);
                    } else if (strcmp(jimp->string, "version") == 0) {
                        if (!jimp_string(jimp)) return false;
                        params->init_params.client_info_ver = jimp_string_as_sv(jimp);
                    } else {
                        if (!jimp_skip_member(jimp)) return false;
                    }
                }
                if (!jimp_object_end(jimp)) return false;
            } else {
                if (!jimp_skip_member(jimp)) return false;
            }
        }
        return jimp_object_end(jimp);
    } else if (sv_eq(method, sv_from_cstr("subtract"))) {
        params->method_type = METHOD_SUBTRACT;
        size_t idx = 0;
        if (!jimp_array_begin(jimp)) return false;
        while (jimp_array_item(jimp)) {
            if (idx >= 2) return false;
            if (!jimp_number(jimp)) return false;
            if (idx == 0) params->subtract_params.x = jimp->number;
            else if (idx == 1) params->subtract_params.y = jimp->number;
            idx++;
        }
        if (!jimp_array_end(jimp)) return false;
        return idx == 2;
    } else if (sv_starts_with(method, sv_from_cstr("notification"))) {
        params->method_type = METHOD_NOTIF;
        return true;
    }
    return false;
}

JSONRPC_Error_Code method_handler(void *ctx, String_View method, void *ptr, Jim *success, Jim *failure, char **error_message) {
    UNUSED(ctx); UNUSED(method); UNUSED(failure); UNUSED(error_message);
    Params *params = ptr;
    switch (params->method_type) {
        case METHOD_INIT:
            jim_string(success, "initialized");
            return JSONRPC_ERROR_CODE_SUCCESS;
        case METHOD_SUBTRACT:
            jim_float(success, params->subtract_params.x - params->subtract_params.y);
            return JSONRPC_ERROR_CODE_SUCCESS;
        case METHOD_NOTIF:
            return JSONRPC_ERROR_CODE_NO_RESPONSE;
        default:
            return JSONRPC_ERROR_CODE_METHOD_NOT_FOUND;
    }
}

// --- FULL TEST HARNESS ---

void run_test(const char *test_name, const char *json_request) {
    nob_log(INFO, "TEST [%s]: Sending -> %s", test_name, json_request);

    // 1. Setup Input (Mock Stdin)
    int in_fd = memfd_create("mock_in", 0);
    write(in_fd, json_request, strlen(json_request));
    lseek(in_fd, 0, SEEK_SET);

    // 2. Setup Output (Mock Stdout)
    int out_fd = memfd_create("mock_out", 0);

    // 3. Setup Library Session
    Params params = {0};
    JSONRPC_Params_Parser p_parser = {.params = &params, .parse_clb = parse_params};
    JSONRPC_Session session = create_jsonrpc_session(in_fd, "in", out_fd, "out", p_parser, method_handler, NULL);

    // 4. Run Library Logic
    if (!jsonrpc_handle_request(&session)) {
        nob_log(WARNING, "Session ended (likely EOF)");
    }

    // 5. Read Back Output
    char response[1024] = {0};
    lseek(out_fd, 0, SEEK_SET);
    read(out_fd, response, sizeof(response) - 1);
    nob_log(INFO, "RESPONSE: %s\n", response);

    // 6. Manual Cleanup (Library doesn't close FDs)
    free_jsonrpc_session(&session);
    close(in_fd);
    close(out_fd);
}

int main(void) {
    // Case 1: Subtract (42 - 23)
    run_test("Subtract Success",
        "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"subtract\",\"params\":[42, 23]}");

    // Case 2: Initialize
    run_test("Initialize Success",
        "{\"jsonrpc\":\"2.0\",\"id\":2,\"method\":\"initialize\",\"params\":{\"protocolVersion\":\"1.0\"}}");

    // Case 3: Parse Error (Missing bracket)
    run_test("Invalid JSON",
        "{\"jsonrpc\":\"2.0\",\"method\":\"subtract\",\"params\":[1, 2");

    return 0;
}

