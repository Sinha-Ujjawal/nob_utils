#ifndef NOB_GRAPH_H_
#define NOB_GRAPH_H_

#include <stdlib.h>

typedef void* Nob_Graph_Node;

typedef struct {
    nob_embed_da(Nob_Graph_Node);
} Nob_Graph_Nodes;

typedef void (*Nob_Graph_Neighbor_Fn) (void *ctx, Nob_Graph_Node node, Nob_Graph_Nodes *neighbors);
typedef size_t (*Nob_Graph_Hash_Node_Fn) (Nob_Graph_Node node);
typedef bool (*Nob_Graph_Is_Node_Eql_Fn) (Nob_Graph_Node node1, Nob_Graph_Node node2);

typedef struct {
    Nob_Graph_Node node;
    Nob_Graph_Node parent;
    bool           has_parent;
    size_t         depth;
} Nob_Graph_Rel;

typedef struct {
    nob_embed_da(Nob_Graph_Rel);
} Nob_Graph_Rels;

typedef bool (*Nob_Graph_Rel_Clb_Fn) (void *ctx, Nob_Graph_Rel rel_node);

void nob_graph_bfs(
    void *ctx,
    Nob_Graph_Node start,
    Nob_Graph_Neighbor_Fn neighbor_fn,
    Nob_Graph_Hash_Node_Fn hash_fn,
    Nob_Graph_Is_Node_Eql_Fn is_eql_fn,
    Nob_Graph_Rel_Clb_Fn clb);

void nob_graph_dfs(
    void *ctx,
    Nob_Graph_Node start,
    Nob_Graph_Neighbor_Fn neighbor_fn,
    Nob_Graph_Hash_Node_Fn hash_fn,
    Nob_Graph_Is_Node_Eql_Fn is_eql_fn,
    Nob_Graph_Rel_Clb_Fn clb);

bool nob_graph_topo_visit(
    void *ctx,
    Nob_Graph_Node start,
    Nob_Graph_Neighbor_Fn neighbor_fn,
    Nob_Graph_Hash_Node_Fn hash_fn,
    Nob_Graph_Is_Node_Eql_Fn is_eql_fn,
    Nob_Graph_Rel_Clb_Fn clb);

bool nob_graph_topo_sort(
    void *ctx,
    Nob_Graph_Node start,
    Nob_Graph_Neighbor_Fn neighbor_fn,
    Nob_Graph_Hash_Node_Fn hash_fn,
    Nob_Graph_Is_Node_Eql_Fn is_eql_fn,
    Nob_Graph_Rels *rels);

bool nob_graph_is_cyclic(
    void *ctx,
    Nob_Graph_Node start,
    Nob_Graph_Neighbor_Fn neighbor_fn,
    Nob_Graph_Hash_Node_Fn hash_fn,
    Nob_Graph_Is_Node_Eql_Fn is_eql_fn);

#endif // NOB_GRAPH_H_

#ifdef NOB_GRAPH_IMPLEMENTATION
#ifndef NOB_GRAPH_IMPLEMENTATION_GAURD_
#define NOB_GRAPH_IMPLEMENTATION_GAURD_

void nob_graph_bfs(
    void *ctx,
    Nob_Graph_Node start,
    Nob_Graph_Neighbor_Fn neighbor_fn,
    Nob_Graph_Hash_Node_Fn hash_fn,
    Nob_Graph_Is_Node_Eql_Fn is_eql_fn,
    Nob_Graph_Rel_Clb_Fn clb) {
    if (clb == NULL) return;
    typedef struct {
        nob_embed_deque(Nob_Graph_Rel);
    } Queue;
    typedef struct {
        nob_embed_ht_key_slot(Nob_Graph_Node);
        bool seen_before;
    } Seen_Set_Key_Slot;
    typedef struct {
        nob_embed_ht_with_slot(Seen_Set_Key_Slot);
    } Seen_Set;
    Queue q = {0};
    Seen_Set seen_set = {0};
    nob_deque_append(&q, ((Nob_Graph_Rel) {
        .node=start,
        .has_parent=false,
        .depth=0
    }));
    size_t slot_idx;
    nob_ht_insert_key(&seen_set, hash_fn, is_eql_fn, start, &slot_idx);
    seen_set.items[slot_idx].seen_before = true;
    Nob_Graph_Nodes neighbors = {0};
    while (q.count > 0) {
        Nob_Graph_Rel bfs_node;
        nob_deque_shift(&q, &bfs_node);
        if (!clb(ctx, bfs_node)) goto cleanup;
        neighbors.count = 0;
        neighbor_fn(ctx, bfs_node.node, &neighbors);
        nob_da_foreach(Nob_Graph_Node, node, &neighbors) {
            nob_ht_insert_key(&seen_set, hash_fn, is_eql_fn, *node, &slot_idx);
            if (seen_set.items[slot_idx].seen_before) continue;
            seen_set.items[slot_idx].seen_before = true;
            nob_deque_append(&q, ((Nob_Graph_Rel) {
                .node = *node,
                .parent = bfs_node.node,
                .has_parent = true,
                .depth = bfs_node.depth + 1,
            }));
        }
    }

cleanup:
    free(q.items);
    free(seen_set.items);
    free(neighbors.items);
}

void nob_graph_dfs(
    void *ctx,
    Nob_Graph_Node start,
    Nob_Graph_Neighbor_Fn neighbor_fn,
    Nob_Graph_Hash_Node_Fn hash_fn,
    Nob_Graph_Is_Node_Eql_Fn is_eql_fn,
    Nob_Graph_Rel_Clb_Fn clb) {
    if (clb == NULL) return;
    typedef struct {
        nob_embed_deque(Nob_Graph_Rel);
    } Stack;
    typedef struct {
        nob_embed_ht_key_slot(Nob_Graph_Node);
        bool seen_before;
    } Seen_Set_Key_Slot;
    typedef struct {
        nob_embed_ht_with_slot(Seen_Set_Key_Slot);
    } Seen_Set;
    Stack s = {0};
    Seen_Set seen_set = {0};
    nob_deque_append(&s, ((Nob_Graph_Rel) {
        .node=start,
        .has_parent=false,
        .depth=0
    }));
    size_t slot_idx;
    nob_ht_insert_key(&seen_set, hash_fn, is_eql_fn, start, &slot_idx);
    seen_set.items[slot_idx].seen_before = true;
    Nob_Graph_Nodes neighbors = {0};
    while (s.count > 0) {
        Nob_Graph_Rel dfs_node;
        nob_deque_pop(&s, &dfs_node);
        if (!clb(ctx, dfs_node)) goto cleanup;
        neighbors.count = 0;
        neighbor_fn(ctx, dfs_node.node, &neighbors);
        nob_da_foreach(Nob_Graph_Node, node, &neighbors) {
            nob_ht_insert_key(&seen_set, hash_fn, is_eql_fn, *node, &slot_idx);
            if (seen_set.items[slot_idx].seen_before) continue;
            seen_set.items[slot_idx].seen_before = true;
            nob_deque_append(&s, ((Nob_Graph_Rel) {
                .node = *node,
                .parent = dfs_node.node,
                .has_parent = true,
                .depth = dfs_node.depth + 1,
            }));
        }
    }

cleanup:
    free(s.items);
    free(seen_set.items);
    free(neighbors.items);
}

bool nob_graph_topo_visit(
    void *ctx,
    Nob_Graph_Node start,
    Nob_Graph_Neighbor_Fn neighbor_fn,
    Nob_Graph_Hash_Node_Fn hash_fn,
    Nob_Graph_Is_Node_Eql_Fn is_eql_fn,
    Nob_Graph_Rel_Clb_Fn clb) {
    bool is_cyclic = false;
    typedef enum {
        CALL,
        HANDLE,
    } Tag;
    typedef struct {
        Tag           tag;
        Nob_Graph_Rel rel;
    } Tagged_Rel;
    typedef struct {
        nob_embed_deque(Tagged_Rel);
    } Stack;
    typedef struct {
        nob_embed_ht_key_slot(Nob_Graph_Node);
        bool seen_before;
        bool in_progress;
    } Seen_Set_Key_Slot;
    typedef struct {
        nob_embed_ht_with_slot(Seen_Set_Key_Slot);
    } Seen_Set;
    Stack s = {0};
    Seen_Set seen_set = {0};
    nob_deque_append(&s, ((Tagged_Rel) {
        .tag=CALL,
        .rel=((Nob_Graph_Rel) {
            .node=start,
            .has_parent=false,
            .depth=0
        })
    }));
    Nob_Graph_Nodes neighbors = {0};
    while (s.count > 0) {
        Tagged_Rel tagged_rel;
        nob_deque_pop(&s, &tagged_rel);
        size_t slot_idx;
        switch (tagged_rel.tag) {
        case CALL: {
            nob_ht_insert_key(&seen_set, hash_fn, is_eql_fn, tagged_rel.rel.node, &slot_idx);
            if (seen_set.items[slot_idx].in_progress) {
                is_cyclic = true;
                goto cleanup;
            }
            if (seen_set.items[slot_idx].seen_before) continue;
            seen_set.items[slot_idx].seen_before = true;
            seen_set.items[slot_idx].in_progress = true;
            nob_deque_append(&s, ((Tagged_Rel) {
                .tag=HANDLE,
                .rel=tagged_rel.rel,
            }));
            neighbors.count = 0;
            neighbor_fn(ctx, tagged_rel.rel.node, &neighbors);
            nob_da_foreach(Nob_Graph_Node, node, &neighbors) {
                nob_deque_append(&s, ((Tagged_Rel) {
                    .tag=CALL,
                    .rel=((Nob_Graph_Rel) {
                        .node=*node,
                        .parent = tagged_rel.rel.node,
                        .has_parent = true,
                        .depth = tagged_rel.rel.depth + 1,
                    })
                }));
            }
        } break;
        case HANDLE: {
            nob_ht_get_key(&seen_set, hash_fn, is_eql_fn, tagged_rel.rel.node, &slot_idx);
            seen_set.items[slot_idx].in_progress = false;
            if(clb != NULL && !clb(ctx, tagged_rel.rel)) goto cleanup;
        } break;
        default:
            UNREACHABLE("Tag");
        }
    }

cleanup:
    free(s.items);
    free(seen_set.items);
    free(neighbors.items);
    return is_cyclic;
}

typedef struct {
    void *ctx;
    Nob_Graph_Rels *rels;
    Nob_Graph_Neighbor_Fn neighbor_fn;
} Nob_Graph__Topo_Sort_Ctx;

static inline void nob_graph__topo_sort_neighbor_fn(void *ctx, Nob_Graph_Node node, Nob_Graph_Nodes *neighbors) {
    Nob_Graph__Topo_Sort_Ctx *topo_sort_ctx = ctx;
    topo_sort_ctx->neighbor_fn(topo_sort_ctx->ctx, node, neighbors);
}

static inline bool nob_graph__topo_sort_clb(void *ctx, Nob_Graph_Rel rel) {
    Nob_Graph__Topo_Sort_Ctx *topo_sort_ctx = ctx;
    nob_da_append(topo_sort_ctx->rels, rel);
    return true;
}

bool nob_graph_topo_sort(
    void *ctx,
    Nob_Graph_Node start,
    Nob_Graph_Neighbor_Fn neighbor_fn,
    Nob_Graph_Hash_Node_Fn hash_fn,
    Nob_Graph_Is_Node_Eql_Fn is_eql_fn,
    Nob_Graph_Rels *rels) {
    int i = rels->count;
    Nob_Graph__Topo_Sort_Ctx topo_sort_ctx = {.ctx = ctx, .rels = rels, .neighbor_fn=neighbor_fn};
    bool is_cyclic = nob_graph_topo_visit(
        (void *) &topo_sort_ctx,
        start,
        nob_graph__topo_sort_neighbor_fn,
        hash_fn,
        is_eql_fn,
        nob_graph__topo_sort_clb
    );
    int j = rels->count - 1;
    while (i < j) {
        nob_swap(Nob_Graph_Rel, rels->items[i], rels->items[j]);
        i++;
        j--;
    }
    return is_cyclic;
}

bool nob_graph_is_cyclic(
    void *ctx,
    Nob_Graph_Node start,
    Nob_Graph_Neighbor_Fn neighbor_fn,
    Nob_Graph_Hash_Node_Fn hash_fn,
    Nob_Graph_Is_Node_Eql_Fn is_eql_fn) {
    return nob_graph_topo_visit(
        ctx,
        start,
        neighbor_fn,
        hash_fn,
        is_eql_fn,
        NULL
    );
}

#endif // NOB_GRAPH_IMPLEMENTATION_GAURD_
#endif // NOB_GRAPH_IMPLEMENTATION

#ifndef NOB_GRAPH_STRIP_PREFIX_GUARD_
#define NOB_GRAPH_STRIP_PREFIX_GUARD_
    #ifndef NOB_UNSTRIP_PREFIX
        #define Graph_Node           Nob_Graph_Node
        #define Graph_Nodes          Nob_Graph_Nodes
        #define Graph_Neighbor_Fn    Nob_Graph_Neighbor_Fn
        #define Graph_Hash_Node_Fn   Nob_Graph_Hash_Node_Fn
        #define Graph_Is_Node_Eql_Fn Nob_Graph_Is_Node_Eql_Fn
        #define Graph_Rel            Nob_Graph_Rel
        #define Graph_Rels           Nob_Graph_Rels
        #define Graph_Rel_Clb_Fn     Nob_Graph_Rel_Clb_Fn
        #define graph_bfs            nob_graph_bfs
        #define graph_dfs            nob_graph_dfs
        #define graph_topo_visit     nob_graph_topo_visit
        #define graph_topo_sort      nob_graph_topo_sort
        #define graph_is_cyclic      nob_graph_is_cyclic
    #endif // NOB_UNSTRIP_PREFIX
#endif // NOB_GRAPH_STRIP_PREFIX_GUARD_
