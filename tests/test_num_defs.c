#include <stdio.h>

#include "num_defs.h"

#define PRINT_RANGE_OF_TYPE(T, modif)             \
    do {                                          \
        printf(#T "_MIN: %" modif "\n", T##_MIN); \
        printf(#T "_MAX: %" modif "\n", T##_MAX); \
    } while(0)

int main(void) {
    printf("Range of types:\n");
    PRINT_RANGE_OF_TYPE(U8 , PRIu8);
    PRINT_RANGE_OF_TYPE(U16, PRIu16);
    PRINT_RANGE_OF_TYPE(U32, PRIu32);
    PRINT_RANGE_OF_TYPE(U64, PRIu64);
    PRINT_RANGE_OF_TYPE(S8 , PRIs8);
    PRINT_RANGE_OF_TYPE(S16, PRIs16);
    PRINT_RANGE_OF_TYPE(S32, PRIs32);
    PRINT_RANGE_OF_TYPE(S64, PRIs64);
    PRINT_RANGE_OF_TYPE(F32, PRIf32);
    PRINT_RANGE_OF_TYPE(F64, PRIf64);
    return 0;
}
