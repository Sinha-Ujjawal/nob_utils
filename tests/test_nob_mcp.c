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
#define NOB_BR_IMPLEMENTATION
#define NOB_JSONRPC_IMPLEMENTATION
#include "nob.h"
#include "jim.h"
#include "jimp.h"
#include "nob_br.h"
#include "nob_jsonrpc.h"

#define NOB_MCP_IMPLEMENTATION
#include "nob_mcp.h"

// --- Callbacks ---
bool tools_list_clb(MCP_Session *session) {
    mcp_begin_tool(session, "get_echo", .desc="Echoes text"); {
        mcp_add_param(session, "text", MCP_PARAM_TYPE_STRING, .desc="input");
    } mcp_end_tool(session);
    mcp_begin_tool(session, "sum", .desc="Sums numbers"); {
        mcp_add_array_param(session, "nums", MCP_PARAM_TYPE_NUMBER, .desc="list");
    } mcp_end_tool(session);
    return true;
}

bool tools_call_clb(MCP_Session *session, String_View tool_name, Jimp *tool_args) {
    String_Builder *sb = session->ctx;
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
        mcp_write_text_content(session, sb->items);
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
        mcp_write_text_content(session, sb->items);
        return true;
    }
    return false;
}

// --- Test Framework ---
void run_mcp_test(const char *cat, const char *name, const char *json, String_Builder *sb) {
    nob_log(INFO, "[%-10s] %s", cat, name);
    int in_fd = memfd_create("in", 0);
    write(in_fd, json, strlen(json));
    lseek(in_fd, 0, SEEK_SET);
    int out_fd = memfd_create("out", 0);

    MCP_Session session = create_mcp_session(in_fd, "in", out_fd, "out", "srv", "1.0", tools_list_clb, tools_call_clb, sb);
    mcp_handle_request(&session);

    char resp[4096] = {0};
    lseek(out_fd, 0, SEEK_SET);
    read(out_fd, resp, sizeof(resp)-1);
    nob_log(INFO, "   -> %s\n", resp[0] ? resp : "(No Response/Notification)");

    free_mcp_session(&session);
    close(in_fd); close(out_fd);
}

int main(void) {
    String_Builder sb = {0};

    // 1. PROTOCOL: Handshake
    run_mcp_test("PROTO", "Initialize",
        "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"initialize\",\"params\":{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{},\"clientInfo\":{\"name\":\"test\",\"version\":\"1\"}}}", &sb);

    // 2. PROTOCOL: Unknown Method
    run_mcp_test("PROTO", "Unknown Method",
        "{\"jsonrpc\":\"2.0\",\"id\":101,\"method\":\"mcp/unknown\",\"params\":{}}", &sb);

    // 3. PROTOCOL: Notification (No ID)
    run_mcp_test("PROTO", "Notification (No ID)",
        "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\",\"params\":{}}", &sb);

    // 4. DISCOVERY: List Tools
    run_mcp_test("DISCO", "List Tools",
        "{\"jsonrpc\":\"2.0\",\"id\":201,\"method\":\"tools/list\",\"params\":{}}", &sb);

    // 5. APP: Valid Tool Call
    run_mcp_test("APP", "Sum (Normal)",
        "{\"jsonrpc\":\"2.0\",\"id\":301,\"method\":\"tools/call\",\"params\":{\"name\":\"sum\",\"arguments\":{\"nums\":[10, 20.5, -5]}}}", &sb);

    // 6. APP: Empty Array
    run_mcp_test("APP", "Sum (Empty Array)",
        "{\"jsonrpc\":\"2.0\",\"id\":302,\"method\":\"tools/call\",\"params\":{\"name\":\"sum\",\"arguments\":{\"nums\":[]}}}", &sb);

    // 7. APP: Unknown Tool
    run_mcp_test("APP", "Call Non-existent Tool",
        "{\"jsonrpc\":\"2.0\",\"id\":303,\"method\":\"tools/call\",\"params\":{\"name\":\"calc_pi\",\"arguments\":{}}}", &sb);

    // 8. DATA: Very Long String (Buffer Stress)
    run_mcp_test("STRESS", "Long String Echo",
        "{\"jsonrpc\":\"2.0\",\"id\":401,\"method\":\"tools/call\",\"params\":{\"name\":\"get_echo\",\"arguments\":{\"text\":\"A long string... repeating... A long string...\"}}}", &sb);

    // 9. DATA: Malformed JSON types
    run_mcp_test("MALFORMED", "Sum with String in Array",
        "{\"jsonrpc\":\"2.0\",\"id\":501,\"method\":\"tools/call\",\"params\":{\"name\":\"sum\",\"arguments\":{\"nums\":[10, \"oops\"]}}}", &sb);

    // 10. MALFORMED: Missing arguments object
    run_mcp_test("MALFORMED", "Missing Arguments Field",
        "{\"jsonrpc\":\"2.0\",\"id\":502,\"method\":\"tools/call\",\"params\":{\"name\":\"sum\"}}", &sb);

    // 11. DATA: Unicode and Special Characters
    run_mcp_test("DATA", "Unicode Text",
        "{\"jsonrpc\":\"2.0\",\"id\":601,\"method\":\"tools/call\",\"params\":{\"name\":\"get_echo\",\"arguments\":{\"text\":\"Hello 🦀 MCP! \\n\\t Newline and Tab\"}}}", &sb);

    // 12. APP: Unknown Method (Not MCP prefixed)
    run_mcp_test("PROTO", "Random Method",
        "{\"jsonrpc\":\"2.0\",\"id\":602,\"method\":\"calculate/everything\",\"params\":{}}", &sb);

    // 13. APP: Tool Call with Extra Arguments
    // (Checking if your parser correctly ignores unknown fields in tools/call)
    run_mcp_test("APP", "Sum with Extra Fields",
        "{\"jsonrpc\":\"2.0\",\"id\":603,\"method\":\"tools/call\",\"params\":{\"name\":\"sum\",\"extra\":\"ignored\",\"arguments\":{\"nums\":[5, 5], \"unused\":true}}}", &sb);

    // 14. DISCO: List Prompts (Likely returns empty or Method Not Found)
    run_mcp_test("DISCO", "List Prompts",
        "{\"jsonrpc\":\"2.0\",\"id\":701,\"method\":\"prompts/list\",\"params\":{}}", &sb);

    // 15. DISCO: List Resources
    run_mcp_test("DISCO", "List Resources",
        "{\"jsonrpc\":\"2.0\",\"id\":702,\"method\":\"resources/list\",\"params\":{}}", &sb);

    // 16. PROTO: Invalid JSON-RPC version
    run_mcp_test("PROTO", "Wrong Version",
        "{\"jsonrpc\":\"1.0\",\"id\":801,\"method\":\"tools/list\",\"params\":{}}", &sb);

    // 17. MALFORMED: Null params
    run_mcp_test("MALFORMED", "Null Params",
        "{\"jsonrpc\":\"2.0\",\"id\":802,\"method\":\"tools/list\",\"params\":null}", &sb);

    free(sb.items);
    return 0;
}

