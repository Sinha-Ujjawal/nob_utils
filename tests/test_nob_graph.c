#define NOB_HASH_IMPLEMENTATION
#define NOB_GRAPH_IMPLEMENTATION
#include "nob.h"
#include "nob_utils.h"

#define GRID_SIZE 5

Graph_Node pack_coords(uint32_t x, uint32_t y) {
    return (Graph_Node)(uintptr_t)(((uint64_t)y << 32) | x);
}

void unpack_coords(Graph_Node node, uint32_t *x, uint32_t *y) {
    uintptr_t val = (uintptr_t)node;
    *x = (uint32_t)(val & 0xFFFFFFFF);
    *y = (uint32_t)(val >> 32);
}

void grid_neighbors(void *ctx, Graph_Node node, Graph_Nodes *neighbors) {
    UNUSED(ctx);
    uint32_t x, y;
    unpack_coords(node, &x, &y);

    typedef struct {
        int dx;
        int dy;
    } Dir;
    static Dir dirs[] = {
        // {.dx = -1, .dy = -1},
        // {.dx = 0 , .dy = -1},
        // {.dx = +1, .dy = -1},
        // {.dx = -1, .dy =  0},
        {.dx = +1, .dy =  0},
        // {.dx = -1, .dy = +1},
        {.dx = 0 , .dy = +1},
        {.dx = +1, .dy = +1},
    };

    for (size_t i = 0; i < ARRAY_LEN(dirs); ++i) {
        int nx = x + dirs[i].dx;
        int ny = y + dirs[i].dy;

        // Keep within Grid bounds
        if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE) {
            da_append(neighbors, pack_coords(nx, ny));
        }
    }
}

size_t grid_hash(Graph_Node node) {
    uint32_t x, y;
    unpack_coords(node, &x, &y);
    return hash_int(x + y * GRID_SIZE);
}

bool grid_is_eql(Graph_Node n1, Graph_Node n2) {
    return n1 == n2;
}

bool grid_callback(void *ctx, Graph_Rel rel_node) {
    uint32_t x, y;
    unpack_coords(rel_node.node, &x, &y);

    printf("Visiting: (%u, %u) at depth %zu\n", x, y, rel_node.depth);

    if (ctx != NULL) {
        // Stop search when we reach (dest_x, dest_y)
        uint32_t dest_x, dest_y;
        unpack_coords((Graph_Node) ctx, &dest_x, &dest_y);
        if (x == dest_x && y == dest_y) {
            printf(">> Found Target (%d,%d)! Stopping search.\n", dest_x, dest_y);
            return false;
        }
    }
    return true;
}

int main() {
    printf("Starting BFS on a %dx%d Grid from (0,0)...\n", GRID_SIZE, GRID_SIZE);
    graph_bfs(
        pack_coords(3, 3),
        pack_coords(0, 0),
        grid_neighbors,
        grid_hash,
        grid_is_eql,
        grid_callback
    );
    printf("---------------------------------------------------------------\n");
    printf("Starting DFS on a %dx%d Grid from (0,0)...\n", GRID_SIZE, GRID_SIZE);
    graph_dfs(
        pack_coords(3, 3),
        pack_coords(0, 0),
        grid_neighbors,
        grid_hash,
        grid_is_eql,
        grid_callback
    );
    printf("---------------------------------------------------------------\n");
    printf("Starting Topo Visit on a %dx%d Grid from (0,0)...\n", GRID_SIZE, GRID_SIZE);
    graph_topo_visit(
        NULL,
        pack_coords(0, 0),
        grid_neighbors,
        grid_hash,
        grid_is_eql,
        grid_callback
    );
    printf("---------------------------------------------------------------\n");
    printf("Calling Topo Sort on a %dx%d Grid from (0,0)...\n", GRID_SIZE, GRID_SIZE);
    Graph_Rels rels = {0};
    graph_topo_sort(
        NULL,
        pack_coords(0, 0),
        grid_neighbors,
        grid_hash,
        grid_is_eql,
        &rels
    );
    da_foreach(Graph_Rel, rel, &rels) {
        uint32_t x, y;
        unpack_coords(rel->node, &x, &y);
        printf("(%d, %d), depth: %ld\n", x, y, rel->depth);
    }
    free(rels.items);
    return 0;
}

