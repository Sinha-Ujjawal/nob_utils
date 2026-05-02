#include <stdio.h>
#include <stdlib.h>

#define NOB_IMPLEMENTATION
#define NOB_BR_IMPLEMENTATION
#include "nob.h"
#include "nob_br.h"

int main(void) {
    // FILE *fp = fopen(__FILE__, "r");
    // assert(fp != NULL);
    // Buffered_Reader br = create_br(fileno(fp));
    Buffered_Reader br = create_br(STDIN_FILENO);
    String_Builder sb = {0};
    while (br_read_line_to_sb(&br, &sb)) {
        if (sb.count > 0) {
            nob_log(INFO, "Read Line: |"SV_Fmt"|", (int) sb.count, sb.items);
            sb.count = 0;
        }
    }
    // fclose(fp);
    free(sb.items);
    return 0;
}
