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

#ifndef NOB_ASSERT
#include <assert.h>
#define NOB_ASSERT assert
#endif // NOB_ASSERT

#include <stdbool.h>

#define NOB_ILIST_FIELDS \
    size_t parent;       \
    size_t nextSibling;  \
    size_t prevSibling;  \
    size_t firstChild;

#define nob_embed_ilist_node \
    struct {                 \
        NOB_ILIST_FIELDS     \
    }

#define nob_ilist_delink(ilist, accessor, idx)                                                   \
    do {                                                                                         \
        size_t _nob_ilist_delink_idx = (idx);                                                    \
        if (_nob_ilist_delink_idx == 0) break;                                                   \
        size_t _nob_ilist_delink_parent = accessor(ilist[_nob_ilist_delink_idx]).parent;         \
        if (_nob_ilist_delink_parent == 0) break;                                                \
        size_t _nob_ilist_delink_prev = accessor(ilist[_nob_ilist_delink_idx]).prevSibling;      \
        size_t _nob_ilist_delink_next = accessor(ilist[_nob_ilist_delink_idx]).nextSibling;      \
                                                                                                 \
        if (_nob_ilist_delink_next == _nob_ilist_delink_idx) {                                   \
            /* singleton */                                                                      \
            accessor(ilist[_nob_ilist_delink_parent]).firstChild = 0;                            \
        } else {                                                                                 \
            if (accessor(ilist[_nob_ilist_delink_parent]).firstChild == _nob_ilist_delink_idx) { \
                accessor(ilist[_nob_ilist_delink_parent]).firstChild = _nob_ilist_delink_next;   \
            }                                                                                    \
            accessor(ilist[_nob_ilist_delink_prev]).nextSibling = _nob_ilist_delink_next;        \
            accessor(ilist[_nob_ilist_delink_next]).prevSibling = _nob_ilist_delink_prev;        \
        }                                                                                        \
                                                                                                 \
        accessor(ilist[_nob_ilist_delink_idx]).parent = 0;                                       \
        accessor(ilist[_nob_ilist_delink_idx]).prevSibling = 0;                                  \
        accessor(ilist[_nob_ilist_delink_idx]).nextSibling = 0;                                  \
    } while(0)

#define nob__ilist_link_as_siblings(ilist, accessor, i, j)                                            \
    do {                                                                                              \
        size_t _nob__ilist_link_as_siblings_i = (i);                                                  \
        size_t _nob__ilist_link_as_siblings_j = (j);                                                  \
        if (_nob__ilist_link_as_siblings_i == 0 || _nob__ilist_link_as_siblings_j == 0) break;        \
        if (_nob__ilist_link_as_siblings_i == _nob__ilist_link_as_siblings_j) break;                  \
        accessor(ilist[_nob__ilist_link_as_siblings_i]).nextSibling = _nob__ilist_link_as_siblings_j; \
        accessor(ilist[_nob__ilist_link_as_siblings_j]).prevSibling = _nob__ilist_link_as_siblings_i; \
    } while(0);

#define nob_ilist_prepend(ilist, accessor, root, idx)                                                                                      \
    do {                                                                                                                                   \
        size_t _nob_ilist_prepend_root = (root);                                                                                           \
        NOB_ASSERT(_nob_ilist_prepend_root > 0);                                                                                           \
        size_t _nob_ilist_prepend_idx  = (idx);                                                                                            \
        NOB_ASSERT(_nob_ilist_prepend_idx > 0);                                                                                            \
        if (_nob_ilist_prepend_root == _nob_ilist_prepend_idx) break;                                                                      \
        nob_ilist_delink(ilist, accessor, _nob_ilist_prepend_idx);                                                                         \
                                                                                                                                           \
        if (accessor(ilist[_nob_ilist_prepend_root]).firstChild == 0) {                                                                    \
            accessor(ilist[_nob_ilist_prepend_idx]).nextSibling = _nob_ilist_prepend_idx;                                                  \
            accessor(ilist[_nob_ilist_prepend_idx]).prevSibling = _nob_ilist_prepend_idx;                                                  \
        } else {                                                                                                                           \
            nob__ilist_link_as_siblings(ilist, accessor, accessor(ilist[accessor(ilist[_nob_ilist_prepend_root]).firstChild]).prevSibling, \
                                             _nob_ilist_prepend_idx);                                                                      \
            nob__ilist_link_as_siblings(ilist, accessor, _nob_ilist_prepend_idx, accessor(ilist[_nob_ilist_prepend_root]).firstChild);     \
        }                                                                                                                                  \
                                                                                                                                           \
        accessor(ilist[_nob_ilist_prepend_idx]).parent = _nob_ilist_prepend_root;                                                          \
        accessor(ilist[_nob_ilist_prepend_root]).firstChild = _nob_ilist_prepend_idx;                                                      \
    } while(0)

#define nob_ilist_shift(ilist, accessor, root)                                                \
    do {                                                                                      \
        size_t _nob_ilist_shift_root = (root);                                                \
        NOB_ASSERT(_nob_ilist_shift_root > 0);                                                \
        nob_ilist_delink(ilist, accessor, accessor(ilist[_nob_ilist_shift_root]).firstChild); \
    } while(0)

#define nob_ilist_append(ilist, accessor, root, idx)                                                                                      \
    do {                                                                                                                                  \
        size_t _nob_ilist_append_root = (root);                                                                                           \
        NOB_ASSERT(_nob_ilist_append_root > 0);                                                                                           \
        size_t _nob_ilist_append_idx  = (idx);                                                                                            \
        NOB_ASSERT(_nob_ilist_append_idx > 0);                                                                                            \
        if (_nob_ilist_append_root == _nob_ilist_append_idx) break;                                                                       \
        nob_ilist_delink(ilist, accessor, _nob_ilist_append_idx);                                                                         \
        if (accessor(ilist[_nob_ilist_append_root]).firstChild == 0) {                                                                    \
            accessor(ilist[_nob_ilist_append_idx]).nextSibling = _nob_ilist_append_idx;                                                   \
            accessor(ilist[_nob_ilist_append_idx]).prevSibling = _nob_ilist_append_idx;                                                   \
            accessor(ilist[_nob_ilist_append_root]).firstChild = _nob_ilist_append_idx;                                                   \
        } else {                                                                                                                          \
            nob__ilist_link_as_siblings(ilist, accessor, accessor(ilist[accessor(ilist[_nob_ilist_append_root]).firstChild]).prevSibling, \
                                             _nob_ilist_append_idx);                                                                      \
            nob__ilist_link_as_siblings(ilist, accessor, _nob_ilist_append_idx, accessor(ilist[_nob_ilist_append_root]).firstChild);      \
        }                                                                                                                                 \
                                                                                                                                          \
        accessor(ilist[_nob_ilist_append_idx]).parent = _nob_ilist_append_root;                                                           \
    } while(0)

#define nob_ilist_pop(ilist, accessor, root)                                                                             \
    do {                                                                                                                 \
        size_t _nob_ilist_pop_root = (root);                                                                             \
        NOB_ASSERT(_nob_ilist_pop_root > 0);                                                                             \
        nob_ilist_delink(ilist, accessor, accessor(ilist[accessor(ilist[_nob_ilist_pop_root]).firstChild]).prevSibling); \
    } while(0)

typedef struct {
    size_t i;
    bool isFirst;
    int brk;
} Nob__Ilist_Iterator;

Nob__Ilist_Iterator nob__ilist_iterator(size_t idx);
void nob__ilist_iterator_update(Nob__Ilist_Iterator *it, size_t newIdx);

#define nob_ilist_foreach(type, it, ilist, accessor, root)                                                                                                  \
    for (Nob__Ilist_Iterator _nob_ilist_foreach_iterator##__COUNTER__ = nob__ilist_iterator(accessor((*ilist)[(root)]).firstChild);                         \
         !_nob_ilist_foreach_iterator##__COUNTER__.brk &&                                                                                                   \
         (root) != 0 &&                                                                                                                                     \
         _nob_ilist_foreach_iterator##__COUNTER__.i != 0 &&                                                                                                 \
         ( _nob_ilist_foreach_iterator##__COUNTER__.isFirst || _nob_ilist_foreach_iterator##__COUNTER__.i != accessor((*ilist)[(root)]).firstChild);        \
         nob__ilist_iterator_update(&_nob_ilist_foreach_iterator##__COUNTER__, accessor((*ilist)[_nob_ilist_foreach_iterator##__COUNTER__.i]).nextSibling)) \
        for (type *it = (_nob_ilist_foreach_iterator##__COUNTER__.brk = 1, &accessor((*ilist)[_nob_ilist_foreach_iterator##__COUNTER__.i]));                \
             it != NULL;                                                                                                                                    \
             it = NULL, _nob_ilist_foreach_iterator##__COUNTER__.brk = 0)

#endif // NOB_ILIST_H_

#ifdef NOB_ILIST_IMPLEMENTATION
#ifndef NOB_ILIST_IMPLEMENTATION_GAURD_
#define NOB_ILIST_IMPLEMENTATION_GAURD_

Nob__Ilist_Iterator nob__ilist_iterator(size_t idx) {
    return (Nob__Ilist_Iterator){.i=(size_t) (idx), .isFirst=true};
}

void nob__ilist_iterator_update(Nob__Ilist_Iterator *it, size_t newIdx) {
    it->i = newIdx;
    it->isFirst = false;
}

#endif // NOB_ILIST_IMPLEMENTATION_GAURD_
#endif // NOB_ILIST_IMPLEMENTATION

#ifndef NOB_ILIST_STRIP_PREFIX_GUARD_
#define NOB_ILIST_STRIP_PREFIX_GUARD_
    #ifndef NOB_UNSTRIP_PREFIX
        #define ILIST_FIELDS     NOB_ILIST_FIELDS
        #define ilist_delink     nob_ilist_delink
        #define ilist_prepend    nob_ilist_prepend
        #define ilist_shift      nob_ilist_shift
        #define ilist_append     nob_ilist_append
        #define ilist_pop        nob_ilist_pop
        #define ilist_foreach    nob_ilist_foreach
        #define embed_ilist_node nob_embed_ilist_node
    #endif // NOB_UNSTRIP_PREFIX
#endif // NOB_ILIST_STRIP_PREFIX_GUARD_
