#define NOB_IMPLEMENTATION
#define NOB_BR_IMPLEMENTATION
#include "nob.h"
#include "nob_br.h"
#include "nob_deque.h"
#include "nob_fixed_deque.h"
#include "nob_channels.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>

typedef char* String;

// Channel definitions
typedef embed_channel(String, 64) String_Chan;

// A unit-type channel used purely as a signal (the String value is ignored).
// channel_alt lets us wait on both out_chan and quit_chan simultaneously,
// so the writer can unblock and exit without closing out_chan prematurely.
typedef embed_channel(bool, 10) Quit_Chan;

typedef struct {
    String_Chan *in;
    Quit_Chan *quit;
    String_Chan *out;
} Worker_Args;

typedef struct {
    String_Chan *out;
    Quit_Chan   *quit;
} Writer_Args;

// Processor Thread: reads raw lines, simulates work, sends processed lines
void *processor_thread(void *arg) {
    Worker_Args *channels = (Worker_Args*)arg;
    while (true) {
        String line;
        bool quit = false;
        bool ok_line, ok_quit;
        int which;
        
        #define PROCESSOR_ARMS(Send, Recv)      \
            Recv(channels->in, &line, &ok_line) \
            Recv(channels->quit, &quit, &ok_quit)

        channel_alt(which, PROCESSOR_ARMS);
        #undef PROCESSOR_ARMS

        if (which == 0) {
            if (!ok_line) break;
            int wait_ms = rand() % 10000;
            usleep(wait_ms * 1000);

            String_Builder sb = {0};
            sb_append_cstr(&sb, line);
            sb_append_cstr(&sb, " [Processed: ");
            char buf[32];
            sprintf(buf, "%dms]", wait_ms);
            sb_append_cstr(&sb, buf);
            sb_append_null(&sb);
            free(line);
            channel_send(channels->out, sb.items);
        } else {
            nob_log(INFO, "Processor received quit signal, quiting...");
            while(true) {
                channel_recv(channels->in, &line, &ok_line);
                if (!ok_line) break;
                free(line);
            }
            break;
        }
    }
    return NULL;
}

// Writer Thread: selects between a processed-line channel and a quit signal.
//
// Without channel_alt, the writer would block forever on out_chan once main
// wants to shut down (out_chan still has pending items or isn't closed yet).
// With channel_alt, it can react to whichever arrives first: a line to print,
// or the quit signal telling it to drain and exit.
void *writer_thread(void *arg) {
    Writer_Args *wargs = (Writer_Args*)arg;

    while (true) {
        String processed_line = NULL;
        bool quit_signal      = false;
        bool   ok_out, ok_quit;
        int    which;

        // Select between out_chan (processed lines) and quit_chan (shutdown).
        // Whichever has data first wins; the other arm is left untouched.
        #define WRITER_ARMS(Send, Recv)                 \
            Recv(wargs->out,  &processed_line, &ok_out) \
            Recv(wargs->quit, &quit_signal,    &ok_quit)

        channel_alt(which, WRITER_ARMS);
        #undef WRITER_ARMS

        if (which == 0) {
            // out_chan fired: print the line
            if (!ok_out) break; // out_chan was closed and empty
            printf("%s\n", processed_line);
            fflush(stdout);
            free(processed_line);
        } else {
            // quit_chan fired: drain any remaining lines then exit
            // (quit_signal is a dummy String, nothing to free for the signal)
            nob_log(INFO, "Writer received quit signal, draining...");

            // Drain whatever is left in out_chan without blocking forever.
            // Since all processors have already been joined by this point,
            // no new items will arrive — we just flush what's buffered.
            bool drain_ok;
            String remaining;
            while (true) {
                channel_recv(wargs->out, &remaining, &drain_ok);
                if (!drain_ok) break;
                free(remaining);
            }
            break;
        }
    }
    return NULL;
}

int main() {
    srand(time(NULL));

    String_Chan raw_chan;
    String_Chan out_chan;
    Quit_Chan   quit_chan;
    channel_init(&raw_chan);
    channel_init(&out_chan);
    channel_init(&quit_chan);

    // Start Writer — passes both out_chan and quit_chan so it can alt between them
    Writer_Args wargs = { .out = &out_chan, .quit = &quit_chan };
    pthread_t writer;
    pthread_create(&writer, NULL, writer_thread, &wargs);

    // Start 10 Processors
    pthread_t processors[10];
    Worker_Args args = { .in = &raw_chan, .quit = &quit_chan, .out = &out_chan };
    for (int i = 0; i < (int)ARRAY_LEN(processors); ++i) {
        pthread_create(&processors[i], NULL, processor_thread, &args);
    }

    // Main Thread: read stdin and feed raw_chan until EOF
    Buffered_Reader br = create_br(STDIN_FILENO);
    while (true) {
        String_Builder sb = {0};
        bool got_line = false;
        while (br_read_line_to_sb(&br, &sb)) {
            if (sb.count > 0) { got_line = true; break; }
        }
        if (!got_line) break; // EOF
        sb_append_null(&sb);
        if (strcmp(sb.items, "quit") == 0) break;
        channel_send(&raw_chan, sb.items);
    }
    channel_close(&raw_chan);
    for (int i = 0; i < (int)ARRAY_LEN(processors); ++i) {
        channel_send(&quit_chan, NULL);
    }
    for (int i = 0; i < (int)ARRAY_LEN(processors); ++i) {
        pthread_join(processors[i], NULL);
    }
    channel_close(&out_chan);
    pthread_join(writer, NULL);

    return 0;
}
