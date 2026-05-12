#ifndef NOB_ENTITY_H_
#define NOB_ENTITY_H_

/*
 * Required dependencies:
 * - nob.h (for da impl)
 * - nob_ilist.h (for intrusive list impl)

 0        is reserved for NULL
 1        is reserved for deleted/free entities
 [2..n+1] will be used for the entity kinds
*/

#define NOB_ENTITY_FREE     1
#define NOB_ENTITY_KIND_OFF 2

#define embed_entitities(T)   \
    struct {                  \
        struct {              \
            T value;          \
            embed_ilist_node; \
        } *items;             \
        size_t count;         \
        size_t capacity;      \
        size_t kinds;         \
    }

#define nob_entity_reset(ent, knds)                                           \
    do {                                                                      \
        (ent)->count = 0;                                                     \
        (ent)->kinds = (knds);                                                \
        nob_da_reserve((ent), (knds) + 2);                                    \
        (ent)->count = (knds) + 2;                                            \
        if ((ent)->items) {                                                   \
            memset((ent)->items, 0, sizeof(*(ent)->items) * (ent)->capacity); \
        }                                                                     \
    } while(0)

#define nob_entity_create(ent, kind, index_ptr)                                     \
    do {                                                                            \
        assert((kind) >= 0 && (kind) < (ent)->kinds && "Invalid kind");             \
        if ((ent)->items[NOB_ENTITY_FREE].firstChild != 0) {                        \
            /* There are some free entity slots */                                  \
            *(index_ptr) = (ent)->items[NOB_ENTITY_FREE].firstChild;                \
            nob_ilist_delink((ent)->items, *(index_ptr));                           \
        } else {                                                                    \
            nob_da_reserve((ent), (ent)->count + 1);                                \
            *(index_ptr) = (ent)->count;                                            \
            memset((ent)->items + *(index_ptr), 0, sizeof(*(ent)->items));          \
            (ent)->count += 1;                                                      \
        }                                                                           \
        /* link the kind with the *(index_ptr) */                                   \
        nob_ilist_append((ent)->items, (kind) + NOB_ENTITY_KIND_OFF, *(index_ptr)); \
    } while(0)

#define nob_entity_get(ent, index, value_ptr)                               \
    do {                                                                    \
        size_t _nob_get_idx = (index);                                      \
        /* Reset pointer to NULL by default */                              \
        (value_ptr) = NULL;                                                 \
        /* Basic bounds check */                                            \
        if (_nob_get_idx >= (ent)->count) break;                            \
        /* Ensure it's a real entity, not a Reserved Head/NULL/Free slot */ \
        if (_nob_get_idx < (ent)->kinds + NOB_ENTITY_KIND_OFF) break;       \
        /* Ensure it is actually linked to a valid Kind */                  \
        size_t _parent = (ent)->items[_nob_get_idx].parent;                 \
        if (_parent >= NOB_ENTITY_KIND_OFF &&                               \
            _parent < (ent)->kinds + NOB_ENTITY_KIND_OFF) {                 \
            (value_ptr) = &(ent)->items[_nob_get_idx].value;                \
        }                                                                   \
    } while(0)

#define nob_entity_get_kind(ent, index, kind)                               \
    do {                                                                    \
        size_t _nob_get_idx = (index);                                      \
        /* Reset kind to -1 */                                              \
        (kind) = -1;                                                        \
        /* Basic bounds check */                                            \
        if (_nob_get_idx >= (ent)->count) break;                            \
        /* Ensure it's a real entity, not a Reserved Head/NULL/Free slot */ \
        if (_nob_get_idx < (ent)->kinds + NOB_ENTITY_KIND_OFF) break;       \
        /* Ensure it is actually linked to a valid Kind */                  \
        size_t _parent = (ent)->items[_nob_get_idx].parent;                 \
        if (_parent >= NOB_ENTITY_KIND_OFF &&                               \
            _parent < (ent)->kinds + NOB_ENTITY_KIND_OFF) {                 \
            (kind) = _parent - NOB_ENTITY_KIND_OFF;                         \
        }                                                                   \
    } while(0)

#define nob_entity_delete(ent, index)                                       \
    do {                                                                    \
        size_t _nob_get_idx = (index);                                      \
        /* Basic bounds check */                                            \
        if (_nob_get_idx >= (ent)->count) break;                            \
        /* Ensure it's a real entity, not a Reserved Head/NULL/Free slot */ \
        if (_nob_get_idx < (ent)->kinds + NOB_ENTITY_KIND_OFF) break;       \
        /* Ensure it is actually linked to a valid Kind */                  \
        size_t _parent = (ent)->items[_nob_get_idx].parent;                 \
        if (_parent >= NOB_ENTITY_KIND_OFF &&                               \
            _parent < (ent)->kinds + NOB_ENTITY_KIND_OFF) {                 \
            nob_ilist_append((ent)->items, NOB_ENTITY_FREE, _nob_get_idx);  \
        }                                                                   \
    } while (0)

#define nob_entity_move(ent, index, new_kind)                                        \
    do {                                                                             \
        assert((new_kind) >= 0 && (new_kind) < (ent)->kinds && "Invalid kind");      \
        size_t _nob_get_idx = (index);                                               \
        /* Basic bounds check */                                                     \
        if (_nob_get_idx >= (ent)->count) break;                                     \
        /* Ensure it's a real entity, not a Reserved Head/NULL/Free slot */          \
        if (_nob_get_idx < (ent)->kinds + NOB_ENTITY_KIND_OFF) break;                \
        /* Ensure it is actually linked to a valid Kind */                           \
        size_t _parent = (ent)->items[_nob_get_idx].parent;                          \
        if (_parent >= NOB_ENTITY_KIND_OFF &&                                        \
            _parent < (ent)->kinds + NOB_ENTITY_KIND_OFF &&                          \
            _parent != (new_kind)) {                                                 \
            /* Delinking from existing kind */                                       \
            nob_ilist_delink((ent)->items, index);                                   \
            /* Linking to new kind */                                                \
            nob_ilist_append((ent)->items, (new_kind) + NOB_ENTITY_KIND_OFF, index); \
        }                                                                            \
    } while(0)

#define nob_entity_foreach(type, it, ent, kind)                                                                                                                                  \
    for (Nob__Ilist_Iterator _nob_ilist_foreach_iterator = nob__ilist_iterator((ent)->items[(kind) + NOB_ENTITY_KIND_OFF].firstChild);                                           \
         _nob_ilist_foreach_iterator.i != 0 && ( _nob_ilist_foreach_iterator.isFirst || _nob_ilist_foreach_iterator.i != (ent)->items[(kind) + NOB_ENTITY_KIND_OFF].firstChild); \
         nob__ilist_iterator_update(&_nob_ilist_foreach_iterator, (ent)->items[_nob_ilist_foreach_iterator.i].nextSibling))                                                      \
        for (type *it = &(ent)->items[_nob_ilist_foreach_iterator.i].value; it != NULL; it = NULL)

#endif // NOB_ENTITY_H_

#ifndef NOB_ENTITY_STRIP_PREFIX_GUARD_
#define NOB_ENTITY_STRIP_PREFIX_GUARD_
    #ifndef NOB_UNSTRIP_PREFIX
        #define entity_reset    nob_entity_reset
        #define entity_create   nob_entity_create
        #define entity_get      nob_entity_get
        #define entity_get_kind nob_entity_get_kind
        #define entity_delete   nob_entity_delete
        #define entity_move     nob_entity_move
        #define entity_foreach  nob_entity_foreach
    #endif // NOB_UNSTRIP_PREFIX
#endif // NOB_ENTITY_STRIP_PREFIX_GUARD_
