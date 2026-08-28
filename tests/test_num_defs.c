#include <stdio.h>

#include "num_defs.h"

#define PRINT_RANGE_OF_TYPE(T, modif)            \
    do {                                         \
        printf(""#T"_MIN: "#modif"\n", T##_MIN); \
        printf(""#T"_MAX: "#modif"\n", T##_MAX); \
    } while(0)                                   \

int main(void) {
    printf("Range of types:\n");
    PRINT_RANGE_OF_TYPE(U8 , %hhu);
    PRINT_RANGE_OF_TYPE(U16, %hu);
    PRINT_RANGE_OF_TYPE(U32, %lu);
    PRINT_RANGE_OF_TYPE(U64, %llu);
    PRINT_RANGE_OF_TYPE(S8 , %hhd);
    PRINT_RANGE_OF_TYPE(S16, %hd);
    PRINT_RANGE_OF_TYPE(S32, %ld);
    PRINT_RANGE_OF_TYPE(S64, %lld);
    PRINT_RANGE_OF_TYPE(F32, %f);
    PRINT_RANGE_OF_TYPE(F64, %lf);
    return 0;
}
