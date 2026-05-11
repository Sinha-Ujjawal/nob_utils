#define _GNU_SOURCE
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define NOB_IMPLEMENTATION
#define JIM_IMPLEMENTATION
#define JIMP_IMPLEMENTATION
#define NOB_JSONRPC_IMPLEMENTATION
#include "nob.h"
#include "jim.h"
#include "jimp.h"
#include "nob_jsonrpc.h"

#define NOB_MCP_IMPLEMENTATION
#include "nob_mcp.h"

// --- Callbacks ---
bool tools_list_clb(MCP_Request_Handler *request_handler) {
    mcp_begin_tool(request_handler, "get_echo", .desc="Echoes text"); {
        mcp_add_param(request_handler, "text", MCP_PARAM_TYPE_STRING, .desc="input");
    } mcp_end_tool(request_handler);
    mcp_begin_tool(request_handler, "sum", .desc="Sums numbers"); {
        mcp_add_array_param(request_handler, "nums", MCP_PARAM_TYPE_NUMBER, .desc="list");
    } mcp_end_tool(request_handler);
    return true;
}

bool tools_call_clb(MCP_Request_Handler *request_handler, String_View tool_name, Jimp *tool_args) {
    String_Builder *sb = request_handler->ctx;
    if (sv_eq(tool_name, sv_from_cstr("get_echo"))) {
        const char *text = NULL;
        if (!jimp_object_begin(tool_args)) return false;
        while (jimp_object_member(tool_args)) {
            if (strcmp(tool_args->string, "text") == 0) {
                if (!jimp_string(tool_args)) return false;
                text = tool_args->string;
            } else jimp_skip_member(tool_args);
        }
        if (!jimp_object_end(tool_args) || !text) return false;
        sb->count = 0; sb_appendf(sb, "Echo: %s", text); sb_append_null(sb);
        mcp_write_text_content(request_handler, sb->items);
        return true;
    } else if (sv_eq(tool_name, sv_from_cstr("sum"))) {
        double total = 0.0;
        if (!jimp_object_begin(tool_args)) return false;
        while (jimp_object_member(tool_args)) {
            if (strcmp(tool_args->string, "nums") == 0) {
                if (!jimp_array_begin(tool_args)) return false;
                while (jimp_array_item(tool_args)) {
                    if (!jimp_number(tool_args)) return false;
                    total += tool_args->number;
                }
                jimp_array_end(tool_args);
            } else jimp_skip_member(tool_args);
        }
        jimp_object_end(tool_args);
        sb->count = 0; sb_appendf(sb, "%f", total); sb_append_null(sb);
        mcp_write_text_content(request_handler, sb->items);
        return true;
    }
    return false;
}

// --- Test Framework ---
void run_mcp_test(const char *cat, const char *name, const char *json, MCP_Request_Handler *req_handler, Jim *success, Jim *failure) {
    nob_log(INFO, "[%-10s] %s", cat, name);
    nob_log(INFO, "<- %s", json);

    bool res = mcp_handle_request(
        req_handler,
        "in", json, strlen(json),
        success, failure);
    String_View response = {0};
    if (res) {
        response = sv_from_parts(success->sink, success->sink_count);
    } else {
        response = sv_from_parts(failure->sink, failure->sink_count);
    }
    nob_log(INFO, "-> "SV_Fmt, SV_Arg(response));
}

int main(void) {
    Jim success = {0};
    Jim failure = {0};
    String_Builder sb = {0};
    MCP_Request_Handler req_handler = create_mcp_request_handler("srv", "1.0", tools_list_clb, tools_call_clb, &sb, NULL);

    // - PROTOCOL: Handshake
    run_mcp_test("PROTO", "Initialize",
        "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"initialize\",\"params\":{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{},\"clientInfo\":{\"name\":\"test\",\"version\":\"1\"}}}", &req_handler, &success, &failure);

    // - PROTOCOL: Handshake (With Instructions)
    req_handler.instructions = "Some dummy instruction";
    run_mcp_test("PROTO", "Initialize (With Instructions)",
        "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"initialize\",\"params\":{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{},\"clientInfo\":{\"name\":\"test\",\"version\":\"1\"}}}", &req_handler, &success, &failure);
    req_handler.instructions = NULL;

    // - PROTOCOL: Unknown Method
    run_mcp_test("PROTO", "Unknown Method",
        "{\"jsonrpc\":\"2.0\",\"id\":101,\"method\":\"mcp/unknown\",\"params\":{}}", &req_handler, &success, &failure);

    // - PROTOCOL: Notification (No ID)
    run_mcp_test("PROTO", "Notification (No ID)",
        "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\",\"params\":{}}", &req_handler, &success, &failure);

    // - DISCOVERY: List Tools
    run_mcp_test("DISCO", "List Tools",
        "{\"jsonrpc\":\"2.0\",\"id\":201,\"method\":\"tools/list\",\"params\":{}}", &req_handler, &success, &failure);

    // - APP: Valid Tool Call
    run_mcp_test("APP", "Sum (Normal)",
        "{\"jsonrpc\":\"2.0\",\"id\":301,\"method\":\"tools/call\",\"params\":{\"name\":\"sum\",\"arguments\":{\"nums\":[10, 20.5, -5]}}}", &req_handler, &success, &failure);

    // - APP: Empty Array
    run_mcp_test("APP", "Sum (Empty Array)",
        "{\"jsonrpc\":\"2.0\",\"id\":302,\"method\":\"tools/call\",\"params\":{\"name\":\"sum\",\"arguments\":{\"nums\":[]}}}", &req_handler, &success, &failure);

    // - APP: Unknown Tool
    run_mcp_test("APP", "Call Non-existent Tool",
        "{\"jsonrpc\":\"2.0\",\"id\":303,\"method\":\"tools/call\",\"params\":{\"name\":\"calc_pi\",\"arguments\":{}}}", &req_handler, &success, &failure);

    // - DATA: Very Long String (Buffer Stress)
    run_mcp_test("STRESS", "Long String Echo",
        "{\"jsonrpc\":\"2.0\",\"id\":401,\"method\":\"tools/call\",\"params\":{\"name\":\"get_echo\",\"arguments\":{\"text\":\"A long string... repeating... A long string...\"}}}", &req_handler, &success, &failure);

    // - DATA: Malformed JSON types
    run_mcp_test("MALFORMED", "Sum with String in Array",
        "{\"jsonrpc\":\"2.0\",\"id\":501,\"method\":\"tools/call\",\"params\":{\"name\":\"sum\",\"arguments\":{\"nums\":[10, \"oops\"]}}}", &req_handler, &success, &failure);

    // - MALFORMED: Missing arguments object
    run_mcp_test("MALFORMED", "Missing Arguments Field",
        "{\"jsonrpc\":\"2.0\",\"id\":502,\"method\":\"tools/call\",\"params\":{\"name\":\"sum\"}}", &req_handler, &success, &failure);

    // - DATA: Unicode and Special Characters
    run_mcp_test("DATA", "Unicode Text",
        "{\"jsonrpc\":\"2.0\",\"id\":601,\"method\":\"tools/call\",\"params\":{\"name\":\"get_echo\",\"arguments\":{\"text\":\"Hello 🦀 MCP! \\n\\t Newline and Tab\"}}}", &req_handler, &success, &failure);

    // - APP: Unknown Method (Not MCP prefixed)
    run_mcp_test("PROTO", "Random Method",
        "{\"jsonrpc\":\"2.0\",\"id\":602,\"method\":\"calculate/everything\",\"params\":{}}", &req_handler, &success, &failure);

    // - APP: Tool Call with Extra Arguments
    // (Checking if your parser correctly ignores unknown fields in tools/call)
    run_mcp_test("APP", "Sum with Extra Fields",
        "{\"jsonrpc\":\"2.0\",\"id\":603,\"method\":\"tools/call\",\"params\":{\"name\":\"sum\",\"extra\":\"ignored\",\"arguments\":{\"nums\":[5, 5], \"unused\":true}}}", &req_handler, &success, &failure);

    // - DISCO: List Prompts (Likely returns empty or Method Not Found)
    run_mcp_test("DISCO", "List Prompts",
        "{\"jsonrpc\":\"2.0\",\"id\":701,\"method\":\"prompts/list\",\"params\":{}}", &req_handler, &success, &failure);

    // - DISCO: List Resources
    run_mcp_test("DISCO", "List Resources",
        "{\"jsonrpc\":\"2.0\",\"id\":702,\"method\":\"resources/list\",\"params\":{}}", &req_handler, &success, &failure);

    // - PROTO: Invalid JSON-RPC version
    run_mcp_test("PROTO", "Wrong Version",
        "{\"jsonrpc\":\"1.0\",\"id\":801,\"method\":\"tools/list\",\"params\":{}}", &req_handler, &success, &failure);

    // - MALFORMED: Null params
    run_mcp_test("MALFORMED", "Null Params",
        "{\"jsonrpc\":\"2.0\",\"id\":802,\"method\":\"tools/list\",\"params\":null}", &req_handler, &success, &failure);

    free(sb.items);
    free_mcp_request_handler(&req_handler);
    
    free(success.sink);
    free(success.scopes);

    free(failure.sink);
    free(failure.scopes);
    return 0;
}

