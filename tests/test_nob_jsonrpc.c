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
    nob_log(INFO, "ptr in parse_params: %p", ptr);
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
        if (!jimp_object_end(jimp)) return false;
        return true;
    } else if (sv_eq(method, sv_from_cstr("subtract"))) {
        params->method_type = METHOD_SUBTRACT;
        size_t idx = 0;
        if (!jimp_array_begin(jimp)) return false;
        while (jimp_array_item(jimp)) {
            if (idx >= 2) {
                nob_log(ERROR, "Expected only two elements in the array, got > 2");
                return false;
            }
            if (!jimp_number(jimp)) return false;
            if (idx == 0) {
                params->subtract_params.x = jimp->number;
            } else if (idx == 1) {
                params->subtract_params.y = jimp->number;
            } else {
                UNREACHABLE("subtract_params");
            }
            idx++;
        }
        if (!jimp_array_end(jimp)) return false;
        if (idx != 2) {
            nob_log(ERROR, "Expected only two elements in the array, got < 2");
            return false;
        }
       return true;
    } else if (sv_starts_with(method, sv_from_cstr("notification"))) {
        params->method_type = METHOD_NOTIF;
        return true;
    }
    nob_log(ERROR, "Unknown Method: '"SV_Fmt"'", SV_Arg(method));
    return false;
}

JSONRPC_Error_Code method_handler(void *ctx, String_View method, void *ptr, Jim *success, Jim *failure, char **error_message) {
    UNUSED(ctx);
    UNUSED(method);
    UNUSED(failure);
    UNUSED(error_message);
    nob_log(INFO, "ptr in method_handler: %p", ptr);
    Params *params = ptr;
    nob_log(INFO, "params->method_type: %d", params->method_type);
    nob_log(INFO, "params->subtract_params.x: %f", params->subtract_params.x);
    nob_log(INFO, "params->subtract_params.y: %f", params->subtract_params.y);
    switch (params->method_type) {
        case METHOD_INIT: {
            jim_string(success, "initialized");
            return JSONRPC_ERROR_CODE_SUCCESS;
        } break;
        case METHOD_SUBTRACT: {
            jim_float(success, params->subtract_params.x - params->subtract_params.y);
            return JSONRPC_ERROR_CODE_SUCCESS;
        } break;
        case METHOD_NOTIF: {
            return JSONRPC_ERROR_CODE_NO_RESPONSE;
        }
        default: {
            return JSONRPC_ERROR_CODE_METHOD_NOT_FOUND;
        }
    }
}

int main(void) {
    int result = 1;
    const char sample_data[] = "{"
            "\"params\":{"
                "\"protocolVersion\":\"2025-11-25\","
                "\"capabilities\":{},"
                "\"clientInfo\":{"
                    "\"name\":\"opencode\","
                    "\"version\":\"1.14.32\""
                "}"
            "},"
            "\"method\":\"initialize\","
            "\"jsonrpc\":\"2.0\","
            "\"id\":12345"
        "}";
    JSONRPC_Request_Parser parser = {0};
    Params params = {0};
    JSONRPC_Params_Parser params_parser = {.params=&params, .parse_clb=parse_params};
    if (!jsonrpc_parse_request(&parser, "sample_data", sample_data, ARRAY_LEN(sample_data), params_parser, NULL)) return_defer(false);
    nob_log(INFO, "Parsed Request:");
    nob_log(INFO, "JSONRPC Ver: '%s'", parser.jsonrpc_ver);
    if (parser.is_id_parsed) {
        nob_log(INFO, "ID: %zu", parser.id);
    }
    nob_log(INFO, "Method: '"SV_Fmt"'", SV_Arg(parser.method));
    if (parser.is_params_parsed) {
        nob_log(INFO, "Params:");
        if (sv_eq(parser.method, sv_from_cstr("initialize"))) {
            nob_log(INFO, "  Protocol Version: '"SV_Fmt"'", SV_Arg(params.init_params.protocol_ver));
            nob_log(INFO, "  Client Name     : '"SV_Fmt"'", SV_Arg(params.init_params.client_info_name));
            nob_log(INFO, "  Client Version  : '"SV_Fmt"'", SV_Arg(params.init_params.client_info_ver));
        } else {
            nob_log(ERROR, "Unknown Method: '"SV_Fmt"'", SV_Arg(parser.method));
        }
    }

    JSONRPC_Session session = create_jsonrpc_session(STDIN_FILENO, "stdin", STDOUT_FILENO, "stdout", params_parser, method_handler, NULL);
    for (;;) {
        if (!jsonrpc_handle_request(&session)) return_defer(false);
    }

    result = 0;
defer:
    free_jsonrpc_request_parser(&parser);
    free_jsonrpc_session(&session);
    return result;
}

