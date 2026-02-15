#ifndef NOB_ILIST_H_
#define NOB_ILIST_H_

// Intrusive list based on the Wookash Podcast: Avoiding Modern C++ | Anton Mikhailov (https://youtu.be/ShSGHb65f3M?si=YCE_b0dQ5RjKJjjX)
// Every intrusive list node is supposed to have below fields
// parent, nextSibling, prevSibling, firstChild and lastChild will be indexes in the array
// 0 would indicate nil
// NOTE that if these indexes move around, this library will break.
// struct {
//     ...
//     size_t parent;
//     size_t nextSibling;
//     size_t prevSibling;
//     size_t firstChild;
//     size_t lastChild;
// }
//     x
//    ---------
//   /   \     \
//  x <-> x <-> x

#include <assert.h>

#define NOB_ILIST_FIELDS \
    size_t parent;       \
    size_t nextSibling;  \
    size_t prevSibling;  \
    size_t firstChild;   \
    size_t lastChild;

#define nob_ilist_delink(arr, idx)                                                                            \
    do {                                                                                                      \
        size_t _nob_ilist_delink_idx = (idx);                                                                 \
        if (_nob_ilist_delink_idx == 0) break;                                                                \
        if (arr[_nob_ilist_delink_idx].parent == 0) break;                                                    \
        if (arr[_nob_ilist_delink_idx].prevSibling == 0) {                                                    \
            /* This must be first child of parent */                                                          \
            arr[arr[_nob_ilist_delink_idx].parent].firstChild = arr[_nob_ilist_delink_idx].nextSibling;       \
            arr[arr[arr[_nob_ilist_delink_idx].parent].firstChild].prevSibling = 0;                           \
        }                                                                                                     \
        if (arr[_nob_ilist_delink_idx].nextSibling == 0) {                                                    \
            /* This must be last child of parent */                                                           \
            arr[arr[_nob_ilist_delink_idx].parent].lastChild = arr[_nob_ilist_delink_idx].prevSibling;        \
            arr[arr[arr[_nob_ilist_delink_idx].parent].lastChild].nextSibling = 0;                            \
        }                                                                                                     \
        if (arr[_nob_ilist_delink_idx].prevSibling != 0 && arr[_nob_ilist_delink_idx].nextSibling != 0) {     \
            /* This must be in the middle */                                                                  \
            arr[arr[_nob_ilist_delink_idx].prevSibling].nextSibling = arr[_nob_ilist_delink_idx].nextSibling; \
            arr[arr[_nob_ilist_delink_idx].nextSibling].prevSibling = arr[_nob_ilist_delink_idx].prevSibling; \
        }                                                                                                     \
        arr[_nob_ilist_delink_idx].parent = 0;                                                                \
        arr[_nob_ilist_delink_idx].prevSibling = 0;                                                           \
        arr[_nob_ilist_delink_idx].nextSibling = 0;                                                           \
    } while(0)

#define nob__ilist_link_as_siblings(arr, i, j)                                                 \
    do {                                                                                       \
        size_t _nob__ilist_link_as_siblings_i = (i);                                           \
        size_t _nob__ilist_link_as_siblings_j = (j);                                           \
        if (_nob__ilist_link_as_siblings_i == 0 || _nob__ilist_link_as_siblings_j == 0) break; \
        if (_nob__ilist_link_as_siblings_i == _nob__ilist_link_as_siblings_j) break;           \
        arr[_nob__ilist_link_as_siblings_i].nextSibling = _nob__ilist_link_as_siblings_j;      \
        arr[_nob__ilist_link_as_siblings_j].prevSibling = _nob__ilist_link_as_siblings_i;      \
    } while(0);

#define nob_ilist_prepend(arr, root, idx)                                                                  \
    do {                                                                                                   \
        size_t _nob_ilist_prepend_root = (root);                                                           \
        assert(_nob_ilist_prepend_root > 0);                                                               \
        size_t _nob_ilist_prepend_idx  = (idx);                                                            \
        assert(_nob_ilist_prepend_idx > 0);                                                                \
        if (_nob_ilist_prepend_root == _nob_ilist_prepend_idx) break;                                      \
        nob_ilist_delink(arr, _nob_ilist_prepend_idx);                                                     \
        nob__ilist_link_as_siblings(arr, _nob_ilist_prepend_idx, arr[_nob_ilist_prepend_root].firstChild); \
        arr[_nob_ilist_prepend_idx].parent = _nob_ilist_prepend_root;                                      \
        arr[_nob_ilist_prepend_root].firstChild = _nob_ilist_prepend_idx;                                  \
        if (arr[_nob_ilist_prepend_root].lastChild == 0) {                                                 \
            arr[_nob_ilist_prepend_root].lastChild = _nob_ilist_prepend_idx;                               \
        }                                                                                                  \
    } while(0)

#define nob_ilist_shift(arr, root)                                    \
    do {                                                              \
        size_t _nob_ilist_shift_root = (root);                        \
        assert(_nob_ilist_shift_root > 0);                            \
        nob_ilist_delink(arr, arr[_nob_ilist_shift_root].firstChild); \
    } while(0)

#define nob_ilist_append(arr, root, idx)                                                                \
    do {                                                                                                \
        size_t _nob_ilist_append_root = (root);                                                         \
        assert(_nob_ilist_append_root > 0);                                                             \
        size_t _nob_ilist_append_idx  = (idx);                                                          \
        assert(_nob_ilist_append_idx > 0);                                                              \
        if (_nob_ilist_append_root == _nob_ilist_append_idx) break;                                     \
        nob_ilist_delink(arr, _nob_ilist_append_idx);                                                   \
        nob__ilist_link_as_siblings(arr, arr[_nob_ilist_append_root].lastChild, _nob_ilist_append_idx); \
        arr[_nob_ilist_append_idx].parent = _nob_ilist_append_root;                                     \
        arr[_nob_ilist_append_root].lastChild = _nob_ilist_append_idx;                                  \
        if (arr[_nob_ilist_append_root].firstChild == 0) {                                              \
            arr[_nob_ilist_append_root].firstChild = _nob_ilist_append_idx;                             \
        }                                                                                               \
    } while(0)

#define nob_ilist_pop(arr, root)                                   \
    do {                                                           \
        size_t _nob_ilist_pop_root = (root);                       \
        assert(_nob_ilist_pop_root > 0);                           \
        nob_ilist_delink(arr, arr[_nob_ilist_pop_root].lastChild); \
    } while(0)

#ifndef NOB_ILIST_STRIP_PREFIX_GUARD_
#define NOB_ILIST_STRIP_PREFIX_GUARD_
    #ifndef NOB_UNSTRIP_PREFIX
        #define ILIST_FIELDS  NOB_ILIST_FIELDS
        #define ilist_delink  nob_ilist_delink
        #define ilist_prepend nob_ilist_prepend
        #define ilist_shift   nob_ilist_shift
        #define ilist_append  nob_ilist_append
        #define ilist_pop     nob_ilist_pop
    #endif // NOB_UNSTRIP_PREFIX
#endif // NOB_ILIST_STRIP_PREFIX_GUARD_

#endif // NOB_ILIST_H_
