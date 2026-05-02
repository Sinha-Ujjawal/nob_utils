#ifndef NOB_UTILS_H_
#define NOB_UTILS_H_

#define nob_embed_da(T)  \
    struct {             \
        T *items;        \
        size_t count;    \
        size_t capacity; \
    }

#include "nob_fa.h"
#include "nob_heapq.h"
#include "nob_deque.h"
#include "nob_hash.h"
#include "nob_ht.h"
#include "nob_ilist.h"
#include "nob_profiler.h"
#include "nob_graph.h"
#include "nob_rc.h"
#include "nob_huge_page_alloc.h"
#include "nob_br.h"
#include "num_defs.h"

#ifndef NOB_UTILS_STRIP_PREFIX_GUARD_
#define NOB_UTILS_STRIP_PREFIX_GUARD_
    #ifndef NOB_UNSTRIP_PREFIX
        #define embed_da nob_embed_da
    #endif // NOB_UNSTRIP_PREFIX
#endif // NOB_UTILS_STRIP_PREFIX_GUARD_

#endif // NOB_UTILS_H_
