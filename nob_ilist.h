#ifndef NOB_ILIST_H_
#define NOB_ILIST_H_

/* Intrusive list based on the Wookash Podcast: Avoiding Modern C++ | Anton Mikhailov (https://youtu.be/ShSGHb65f3M?si=YCE_b0dQ5RjKJjjX)
   Every intrusive list node is supposed to have below fields
   parent, nextSibling, prevSibling and firstChild will be indexes in the array
   0 would indicate nil
   NOTE that if these indexes move around, this library will break.
   NOTE that the sibling pointers forms a loop
   struct {
       ...
       size_t parent;
       size_t nextSibling;
       size_t prevSibling;
       size_t firstChild;
   }
       x
      ---------
     /   \     \
    x <-> x <-> x
*/

#include <assert.h>
#include <stdbool.h>

#define NOB_ILIST_FIELDS \
    size_t parent;       \
    size_t nextSibling;  \
    size_t prevSibling;  \
    size_t firstChild;

#define nob_ilist_delink(ilist, idx)                                                   \
    do {                                                                               \
        size_t _nob_ilist_delink_idx = (idx);                                          \
        if (_nob_ilist_delink_idx == 0) break;                                         \
        size_t _nob_ilist_delink_parent = ilist[_nob_ilist_delink_idx].parent;         \
        if (_nob_ilist_delink_parent == 0) break;                                      \
        size_t _nob_ilist_delink_prev = ilist[_nob_ilist_delink_idx].prevSibling;      \
        size_t _nob_ilist_delink_next = ilist[_nob_ilist_delink_idx].nextSibling;      \
                                                                                       \
        if (_nob_ilist_delink_next == _nob_ilist_delink_idx) {                         \
            /* singleton */                                                            \
            ilist[_nob_ilist_delink_parent].firstChild = 0;                            \
        } else {                                                                       \
            if (ilist[_nob_ilist_delink_parent].firstChild == _nob_ilist_delink_idx) { \
                ilist[_nob_ilist_delink_parent].firstChild = _nob_ilist_delink_next;   \
            }                                                                          \
            ilist[_nob_ilist_delink_prev].nextSibling = _nob_ilist_delink_next;        \
            ilist[_nob_ilist_delink_next].prevSibling = _nob_ilist_delink_prev;        \
        }                                                                              \
                                                                                       \
        ilist[_nob_ilist_delink_idx].parent = 0;                                       \
        ilist[_nob_ilist_delink_idx].prevSibling = 0;                                  \
        ilist[_nob_ilist_delink_idx].nextSibling = 0;                                  \
    } while(0)

#define nob__ilist_link_as_siblings(ilist, i, j)                                               \
    do {                                                                                       \
        size_t _nob__ilist_link_as_siblings_i = (i);                                           \
        size_t _nob__ilist_link_as_siblings_j = (j);                                           \
        if (_nob__ilist_link_as_siblings_i == 0 || _nob__ilist_link_as_siblings_j == 0) break; \
        if (_nob__ilist_link_as_siblings_i == _nob__ilist_link_as_siblings_j) break;           \
        ilist[_nob__ilist_link_as_siblings_i].nextSibling = _nob__ilist_link_as_siblings_j;    \
        ilist[_nob__ilist_link_as_siblings_j].prevSibling = _nob__ilist_link_as_siblings_i;    \
    } while(0);

#define nob_ilist_prepend(ilist, root, idx)                                                                        \
    do {                                                                                                           \
        size_t _nob_ilist_prepend_root = (root);                                                                   \
        assert(_nob_ilist_prepend_root > 0);                                                                       \
        size_t _nob_ilist_prepend_idx  = (idx);                                                                    \
        assert(_nob_ilist_prepend_idx > 0);                                                                        \
        if (_nob_ilist_prepend_root == _nob_ilist_prepend_idx) break;                                              \
        nob_ilist_delink(ilist, _nob_ilist_prepend_idx);                                                           \
                                                                                                                   \
        if (ilist[_nob_ilist_prepend_root].firstChild == 0) {                                                      \
            ilist[_nob_ilist_prepend_idx].nextSibling = _nob_ilist_prepend_idx;                                    \
            ilist[_nob_ilist_prepend_idx].prevSibling = _nob_ilist_prepend_idx;                                    \
        } else {                                                                                                   \
            nob__ilist_link_as_siblings(ilist, ilist[ilist[_nob_ilist_prepend_root].firstChild].prevSibling,       \
                                             _nob_ilist_prepend_idx);                                              \
            nob__ilist_link_as_siblings(ilist, _nob_ilist_prepend_idx, ilist[_nob_ilist_prepend_root].firstChild); \
        }                                                                                                          \
                                                                                                                   \
        ilist[_nob_ilist_prepend_idx].parent = _nob_ilist_prepend_root;                                            \
        ilist[_nob_ilist_prepend_root].firstChild = _nob_ilist_prepend_idx;                                        \
    } while(0)

#define nob_ilist_shift(ilist, root)                                      \
    do {                                                                  \
        size_t _nob_ilist_shift_root = (root);                            \
        assert(_nob_ilist_shift_root > 0);                                \
        nob_ilist_delink(ilist, ilist[_nob_ilist_shift_root].firstChild); \
    } while(0)

#define nob_ilist_append(ilist, root, idx)                                                                       \
    do {                                                                                                         \
        size_t _nob_ilist_append_root = (root);                                                                  \
        assert(_nob_ilist_append_root > 0);                                                                      \
        size_t _nob_ilist_append_idx  = (idx);                                                                   \
        assert(_nob_ilist_append_idx > 0);                                                                       \
        if (_nob_ilist_append_root == _nob_ilist_append_idx) break;                                              \
        nob_ilist_delink(ilist, _nob_ilist_append_idx);                                                          \
        if (ilist[_nob_ilist_append_root].firstChild == 0) {                                                     \
            ilist[_nob_ilist_append_idx].nextSibling = _nob_ilist_append_idx;                                    \
            ilist[_nob_ilist_append_idx].prevSibling = _nob_ilist_append_idx;                                    \
            ilist[_nob_ilist_append_root].firstChild = _nob_ilist_append_idx;                                    \
        } else {                                                                                                 \
            nob__ilist_link_as_siblings(ilist, ilist[ilist[_nob_ilist_append_root].firstChild].prevSibling,      \
                                             _nob_ilist_append_idx);                                             \
            nob__ilist_link_as_siblings(ilist, _nob_ilist_append_idx, ilist[_nob_ilist_append_root].firstChild); \
        }                                                                                                        \
                                                                                                                 \
        ilist[_nob_ilist_append_idx].parent = _nob_ilist_append_root;                                            \
    } while(0)

#define nob_ilist_pop(ilist, root)                                                         \
    do {                                                                                   \
        size_t _nob_ilist_pop_root = (root);                                               \
        assert(_nob_ilist_pop_root > 0);                                                   \
        nob_ilist_delink(ilist, ilist[ilist[_nob_ilist_pop_root].firstChild].prevSibling); \
    } while(0)

typedef struct {
    size_t i;
    bool isFirst;
} Nob__Ilist_Iterator;

Nob__Ilist_Iterator nob__ilist_iterator(size_t idx);
void nob__ilist_iterator_update(Nob__Ilist_Iterator *it, size_t newIdx);

#ifdef NOB_ILIST_IMPLEMENTATION
Nob__Ilist_Iterator nob__ilist_iterator(size_t idx) {
    return (Nob__Ilist_Iterator){.i=(size_t) (idx), .isFirst=true};
}

void nob__ilist_iterator_update(Nob__Ilist_Iterator *it, size_t newIdx) {
    it->i = newIdx;
    it->isFirst = false;
}

#define nob_ilist_foreach(type, it, ilist, root)                                                                                                                      \
    for (Nob__Ilist_Iterator _nob_ilist_foreach_iterator = nob__ilist_iterator((*ilist)[(root)].firstChild);                                                          \
         (root) != 0 && _nob_ilist_foreach_iterator.i != 0 && ( _nob_ilist_foreach_iterator.isFirst || _nob_ilist_foreach_iterator.i != (*ilist)[(root)].firstChild); \
         nob__ilist_iterator_update(&_nob_ilist_foreach_iterator, (*ilist)[_nob_ilist_foreach_iterator.i].nextSibling))                                               \
        for (type *it = &(*ilist)[_nob_ilist_foreach_iterator.i]; it != NULL; it = NULL)

#endif // NOB_ILIST_IMPLEMENTATION

#ifndef NOB_ILIST_STRIP_PREFIX_GUARD_
#define NOB_ILIST_STRIP_PREFIX_GUARD_
    #ifndef NOB_UNSTRIP_PREFIX
        #define ILIST_FIELDS  NOB_ILIST_FIELDS
        #define ilist_delink  nob_ilist_delink
        #define ilist_prepend nob_ilist_prepend
        #define ilist_shift   nob_ilist_shift
        #define ilist_append  nob_ilist_append
        #define ilist_pop     nob_ilist_pop
        #define ilist_foreach nob_ilist_foreach
    #endif // NOB_UNSTRIP_PREFIX
#endif // NOB_ILIST_STRIP_PREFIX_GUARD_

#endif // NOB_ILIST_H_
