#define _GNU_SOURCE
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define NOB_IMPLEMENTATION
#define NOB_BR_IMPLEMENTATION
#include "nob.h"
#include "nob_br.h"

int create_mem_fd(const char *name, size_t size) {
    // 1. Create the anonymous file in RAM
    int fd = memfd_create(name, 0);
    if (fd == -1) return -1;

    // 2. Set the initial size (otherwise it's 0)
    if (ftruncate(fd, size) == -1) {
        close(fd);
        return -1;
    }

    return fd;
}

typedef struct {
    int fdin;
} My_Args;

void *thread_read_from_fdin(void *args) {
    My_Args *my_args = args;
    Buffered_Reader br = create_br(my_args->fdin);
    String_Builder sb = {0};
    while (br_read_line_to_sb(&br, &sb)) {
        if (sb.count > 0) {
            nob_log(INFO, "Read Line: |"SV_Fmt"|", (int) sb.count, sb.items);
            if (sv_eq(sb_to_sv(sb), sv_from_cstr("quit"))) {
                nob_log(INFO, "Received `quit`. Quiting!");
                break;
            }
            sb.count = 0;
        }
    }
    free(sb.items);
    return NULL;
}

int main(void) {
    int pipefds[2];
    if (pipe(pipefds) == -1) {\
        nob_log(ERROR, "Could not create pipe: %s", strerror(errno));
        return 1;
    }

    int read_fd = pipefds[0];
    int write_fd = pipefds[1];

    pthread_t read_from_fdin_thread = {0};
    My_Args args = {.fdin = read_fd};
    pthread_create(&read_from_fdin_thread, NULL, thread_read_from_fdin, &args);
    char buffer[512];
    for (size_t i = 0; i < 5; i++) {
        size_t n = snprintf(buffer, sizeof(buffer), "%zu\n", i);
        if (write(write_fd, buffer, n) < 0) {
            nob_log(ERROR, "Could not write to fd: %s", strerror(errno));
            return 1;
        }
    }
    write(write_fd, "quit\n", strlen("quit\n"));
    close(write_fd);
    pthread_join(read_from_fdin_thread, NULL);
    close(read_fd);
    return 0;
}
