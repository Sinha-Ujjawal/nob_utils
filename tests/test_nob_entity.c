#define NOB_IMPLEMENTATION
#define NOB_ILIST_IMPLEMENTATION
#include "nob.h"
#include "nob_ilist.h"
#include "nob_entity.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ── helpers ─────────────────────────────────────────────────────────────── */

static int g_passed = 0;
static int g_failed = 0;

#define TEST(name) static void test_##name(void)
#define RUN(name)  do { printf("  %-55s\n", #name); test_##name(); if (g_failed > 0) goto end; } while(0)
#define SECTION(s) printf("\n[%s]\n", s)

#define EXPECT(cond) \
    do { \
        if (cond) { \
            printf("    PASS\n"); g_passed++; \
        } else { \
            printf("    FAIL  (%s:%d)  %s\n", __FILE__, __LINE__, #cond); \
            g_failed++; \
        } \
    } while(0)

/* ── entity types ────────────────────────────────────────────────────────── */

typedef struct { int x; int y; } Vec2;

#define KIND_VEC2    0
#define KIND_MONSTER 1
#define NUM_KINDS    2

typedef embed_entitities(Vec2) SimpleStore;

/* ── Initialisation ──────────────────────────────────────────────────────── */

TEST(reset_initialises_counts) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    EXPECT(s.count == (size_t)(NUM_KINDS + NOB_ENTITY_KIND_OFF));
    EXPECT(s.kinds  == NUM_KINDS);
}

TEST(reset_clears_free_list) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    EXPECT(s.items[NOB_ENTITY_FREE].firstChild == 0);
}

/* ── Create ──────────────────────────────────────────────────────────────── */

TEST(create_returns_valid_index) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t idx = 0;
    entity_create(&s, KIND_VEC2, &idx);
    EXPECT(idx >= (size_t)(NUM_KINDS + NOB_ENTITY_KIND_OFF));
}

TEST(create_increments_count) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t before = s.count;
    size_t idx = 0;
    entity_create(&s, KIND_VEC2, &idx);
    EXPECT(s.count == before + 1);
}

/* ── Get ─────────────────────────────────────────────────────────────────── */

TEST(get_returns_nonnull_after_create) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t idx = 0;
    entity_create(&s, KIND_VEC2, &idx);
    Vec2 *v = NULL;
    entity_get(&s, idx, v);
    EXPECT(v != NULL);
}

TEST(get_value_survives_write) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t idx = 0;
    entity_create(&s, KIND_VEC2, &idx);
    Vec2 *v = NULL;
    entity_get(&s, idx, v);
    v->x = 42; v->y = 99;
    Vec2 *v2 = NULL;
    entity_get(&s, idx, v2);
    EXPECT(v2 != NULL && v2->x == 42 && v2->y == 99);
}

TEST(get_returns_null_for_index_zero) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    Vec2 *v = NULL;
    entity_get(&s, 0, v);
    EXPECT(v == NULL);
}

TEST(get_returns_null_for_free_head) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    Vec2 *v = NULL;
    entity_get(&s, NOB_ENTITY_FREE, v);
    EXPECT(v == NULL);
}

TEST(get_returns_null_for_kind_head) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    Vec2 *v = NULL;
    entity_get(&s, KIND_VEC2 + NOB_ENTITY_KIND_OFF, v);
    EXPECT(v == NULL);
}

TEST(get_returns_null_for_out_of_bounds) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    Vec2 *v = NULL;
    entity_get(&s, 99999, v);
    EXPECT(v == NULL);
}

/* ── Get kind ────────────────────────────────────────────────────────────── */

TEST(get_kind_correct_after_create) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t idx = 0;
    entity_create(&s, KIND_VEC2, &idx);
    int k = -1;
    entity_get_kind(&s, idx, k);
    EXPECT(k == KIND_VEC2);
}

TEST(get_kind_returns_minus1_for_null) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    int k = 99;
    entity_get_kind(&s, 0, k);
    EXPECT(k == -1);
}

TEST(get_kind_returns_minus1_for_oob) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    int k = 99;
    entity_get_kind(&s, 99999, k);
    EXPECT(k == -1);
}

/* ── Delete ──────────────────────────────────────────────────────────────── */

TEST(delete_makes_get_return_null) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t idx = 0;
    entity_create(&s, KIND_VEC2, &idx);
    entity_delete(&s, idx);
    Vec2 *v = NULL;
    entity_get(&s, idx, v);
    EXPECT(v == NULL);
}

TEST(delete_makes_get_kind_return_minus1) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t idx = 0;
    entity_create(&s, KIND_VEC2, &idx);
    entity_delete(&s, idx);
    int k = 99;
    entity_get_kind(&s, idx, k);
    EXPECT(k == -1);
}

TEST(delete_slot_reused_on_next_create) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t a = 0, b = 0;
    entity_create(&s, KIND_VEC2, &a);
    entity_delete(&s, a);
    size_t count_before = s.count;
    entity_create(&s, KIND_VEC2, &b);
    EXPECT(s.count == count_before);
    EXPECT(b == a);
}

TEST(delete_noop_on_zero) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    entity_delete(&s, 0);
    EXPECT(1);
}

TEST(delete_noop_on_oob) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    entity_delete(&s, 99999);
    EXPECT(1);
}

TEST(delete_noop_on_already_deleted) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t idx = 0;
    entity_create(&s, KIND_VEC2, &idx);
    entity_delete(&s, idx);
    entity_delete(&s, idx);
    EXPECT(1);
}

/* ── Multiple kinds ──────────────────────────────────────────────────────── */

TEST(multiple_kinds_independent) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t vi = 0, mi = 0;
    entity_create(&s, KIND_VEC2,    &vi);
    entity_create(&s, KIND_MONSTER, &mi);
    int kv = -1, km = -1;
    entity_get_kind(&s, vi, kv);
    entity_get_kind(&s, mi, km);
    EXPECT(kv == KIND_VEC2 && km == KIND_MONSTER);
}

TEST(multiple_kinds_foreach_independent) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t idx = 0;
    for (int i = 0; i < 3; i++) {
        entity_create(&s, KIND_VEC2, &idx);
        Vec2 *v = NULL;
        entity_get(&s, idx, v);
        if (v) v->x = 1;
    }
    for (int i = 0; i < 5; i++) {
        entity_create(&s, KIND_MONSTER, &idx);
        Vec2 *v = NULL;
        entity_get(&s, idx, v);
        if (v) v->x = 2;
    }
    int vec2_count = 0, monster_count = 0;
    entity_foreach(Vec2, it, &s, KIND_VEC2)    { vec2_count++;    (void)it; }
    entity_foreach(Vec2, it, &s, KIND_MONSTER) { monster_count++; (void)it; }
    EXPECT(vec2_count == 3 && monster_count == 5);
}

/* ── Foreach ─────────────────────────────────────────────────────────────── */

TEST(foreach_visits_all_created) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    for (int i = 0; i < 5; i++) {
        size_t idx = 0;
        entity_create(&s, KIND_VEC2, &idx);
        Vec2 *v = NULL;
        entity_get(&s, idx, v);
        if (v) { v->x = i + 1; v->y = -(i + 1); }
    }
    int visited = 0;
    int sum_x   = 0;
    entity_foreach(Vec2, it, &s, KIND_VEC2) {
        visited++;
        sum_x += it->x;
    }
    /* 1+2+3+4+5 = 15 */
    EXPECT(visited == 5 && sum_x == 15);
}

TEST(foreach_skips_deleted) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t ids[4];
    for (int i = 0; i < 4; i++) {
        entity_create(&s, KIND_VEC2, &ids[i]);
        Vec2 *v = NULL;
        entity_get(&s, ids[i], v);
        if (v) v->x = i + 1;
    }
    entity_delete(&s, ids[1]); /* x=2 */
    entity_delete(&s, ids[3]); /* x=4 */
    int visited = 0;
    int sum_x   = 0;
    entity_foreach(Vec2, it, &s, KIND_VEC2) {
        visited++;
        sum_x += it->x;
    }
    /* only x=1 and x=3 remain */
    EXPECT(visited == 2 && sum_x == 4);
}

TEST(foreach_empty_store) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    int visited = 0;
    entity_foreach(Vec2, it, &s, KIND_VEC2) {
        visited++;
        (void)it;
    }
    EXPECT(visited == 0);
}

TEST(foreach_after_delete_all) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t ids[3];
    for (int i = 0; i < 3; i++) entity_create(&s, KIND_VEC2, &ids[i]);
    for (int i = 0; i < 3; i++) entity_delete(&s, ids[i]);
    int visited = 0;
    entity_foreach(Vec2, it, &s, KIND_VEC2) {
        visited++;
        (void)it;
    }
    EXPECT(visited == 0);
}

TEST(foreach_modifies_values_in_place) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t ids[3];
    for (int i = 0; i < 3; i++) {
        entity_create(&s, KIND_VEC2, &ids[i]);
        Vec2 *v = NULL;
        entity_get(&s, ids[i], v);
        if (v) { v->x = 0; v->y = 0; }
    }
    entity_foreach(Vec2, it, &s, KIND_VEC2) {
        it->x = 42;
    }
    int all_42 = 1;
    for (int i = 0; i < 3; i++) {
        Vec2 *v = NULL;
        entity_get(&s, ids[i], v);
        if (!v || v->x != 42) { all_42 = 0; break; }
    }
    EXPECT(all_42);
}

/* ── Stress ──────────────────────────────────────────────────────────────── */

#define STRESS_N 100000

TEST(stress_create_all) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    int ok = 1;
    for (int i = 0; i < STRESS_N; i++) {
        size_t idx = 0;
        entity_create(&s, KIND_VEC2, &idx);
        Vec2 *v = NULL;
        entity_get(&s, idx, v);
        if (!v) { ok = 0; break; }
        v->x = i; v->y = -i;
    }
    EXPECT(ok);
    free(s.items);
}

TEST(stress_create_delete_interleaved) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t ids[1000];
    for (int i = 0; i < 1000; i++) entity_create(&s, KIND_VEC2, &ids[i]);
    for (int i = 1; i < 1000; i += 2) entity_delete(&s, ids[i]);
    size_t peak = s.count;
    for (int i = 0; i < 500; i++) {
        size_t idx = 0;
        entity_create(&s, KIND_VEC2, &idx);
        (void)idx;
    }
    EXPECT(s.count == peak);
    free(s.items);
}

TEST(stress_foreach_count) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t all_ids[10000];
    for (int i = 0; i < 10000; i++) {
        entity_create(&s, KIND_VEC2, &all_ids[i]);
    }
    int del = 0;
    for (int i = 0; i < 10000; i += 3) {
        entity_delete(&s, all_ids[i]);
        del++;
    }
    int visited = 0;
    entity_foreach(Vec2, it, &s, KIND_VEC2) {
        visited++;
        (void)it;
    }
    EXPECT(visited == 10000 - del);
    free(s.items);
}

TEST(stress_reset_reuse) {
    SimpleStore s = {0};
    int ok = 1;
    for (int round = 0; round < 10; round++) {
        entity_reset(&s, NUM_KINDS);
        for (int i = 0; i < 1000; i++) {
            size_t idx = 0;
            entity_create(&s, KIND_VEC2, &idx);
            Vec2 *v = NULL;
            entity_get(&s, idx, v);
            if (!v) { ok = 0; goto done; }
            v->x = round * 1000 + i;
        }
        int n = 0;
        entity_foreach(Vec2, it, &s, KIND_VEC2) { n++; (void)it; }
        if (n != 1000) { ok = 0; goto done; }
    }
done:
    EXPECT(ok);
    free(s.items);
}

/* ── Boundary / adversarial ──────────────────────────────────────────────── */

TEST(boundary_single_kind) {
    SimpleStore s = {0};
    entity_reset(&s, 1);
    size_t idx = 0;
    entity_create(&s, 0, &idx);
    Vec2 *v = NULL;
    entity_get(&s, idx, v);
    EXPECT(v != NULL);
    free(s.items);
}

TEST(boundary_index_one_past_kinds) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    Vec2 *v = NULL;
    entity_get(&s, NUM_KINDS + NOB_ENTITY_KIND_OFF - 1, v);
    EXPECT(v == NULL);
}

TEST(boundary_get_kind_for_free_slot) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t idx = 0;
    entity_create(&s, KIND_VEC2, &idx);
    entity_delete(&s, idx);
    int k = 99;
    entity_get_kind(&s, idx, k);
    EXPECT(k == -1);
}

/* ── Move ────────────────────────────────────────────────────────────────── */

TEST(move_changes_kind) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t idx = 0;
    entity_create(&s, KIND_VEC2, &idx);
    entity_move(&s, idx, KIND_MONSTER);
    int k = -1;
    entity_get_kind(&s, idx, k);
    EXPECT(k == KIND_MONSTER);
}

TEST(move_entity_still_accessible) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t idx = 0;
    entity_create(&s, KIND_VEC2, &idx);
    Vec2 *v = NULL;
    entity_get(&s, idx, v);
    v->x = 7; v->y = 8;
    entity_move(&s, idx, KIND_MONSTER);
    Vec2 *v2 = NULL;
    entity_get(&s, idx, v2);
    EXPECT(v2 != NULL && v2->x == 7 && v2->y == 8);
}

TEST(move_appears_in_new_kind_foreach) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t idx = 0;
    entity_create(&s, KIND_VEC2, &idx);
    entity_move(&s, idx, KIND_MONSTER);
    int found = 0;
    entity_foreach(Vec2, it, &s, KIND_MONSTER) {
        if (it == &s.items[idx].value) found++;
    }
    EXPECT(found == 1);
}

TEST(move_absent_from_old_kind_foreach) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t idx = 0;
    entity_create(&s, KIND_VEC2, &idx);
    entity_move(&s, idx, KIND_MONSTER);
    int visited = 0;
    entity_foreach(Vec2, it, &s, KIND_VEC2) { visited++; (void)it; }
    EXPECT(visited == 0);
}

TEST(move_noop_same_kind) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t idx = 0;
    entity_create(&s, KIND_VEC2, &idx);
    entity_move(&s, idx, KIND_VEC2);
    int k = -1;
    entity_get_kind(&s, idx, k);
    EXPECT(k == KIND_VEC2);
    int visited = 0;
    entity_foreach(Vec2, it, &s, KIND_VEC2) { visited++; (void)it; }
    EXPECT(visited == 1);
}

TEST(move_noop_on_deleted) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t idx = 0;
    entity_create(&s, KIND_VEC2, &idx);
    entity_delete(&s, idx);
    entity_move(&s, idx, KIND_MONSTER);
    int k = -1;
    entity_get_kind(&s, idx, k);
    EXPECT(k == -1);
    int monster_count = 0;
    entity_foreach(Vec2, it, &s, KIND_MONSTER) { monster_count++; (void)it; }
    EXPECT(monster_count == 0);
}

TEST(move_noop_on_oob) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    entity_move(&s, 99999, KIND_MONSTER);
    EXPECT(1);
}

TEST(move_noop_on_zero) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    entity_move(&s, 0, KIND_MONSTER);
    EXPECT(1);
}

TEST(move_preserves_other_entities) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t ids[3];
    for (int i = 0; i < 3; i++) {
        entity_create(&s, KIND_VEC2, &ids[i]);
        Vec2 *v = NULL;
        entity_get(&s, ids[i], v);
        if (v) v->x = i + 1;
    }
    entity_move(&s, ids[1], KIND_MONSTER);
    int vec2_count = 0, sum_x = 0;
    entity_foreach(Vec2, it, &s, KIND_VEC2) { vec2_count++; sum_x += it->x; }
    EXPECT(vec2_count == 2 && sum_x == 4); /* x=1 + x=3 */
    int monster_count = 0;
    entity_foreach(Vec2, it, &s, KIND_MONSTER) { monster_count++; (void)it; }
    EXPECT(monster_count == 1);
}

TEST(move_back_and_forth) {
    SimpleStore s = {0};
    entity_reset(&s, NUM_KINDS);
    size_t idx = 0;
    entity_create(&s, KIND_VEC2, &idx);
    entity_move(&s, idx, KIND_MONSTER);
    entity_move(&s, idx, KIND_VEC2);
    int k = -1;
    entity_get_kind(&s, idx, k);
    EXPECT(k == KIND_VEC2);
    int vec2_count = 0, monster_count = 0;
    entity_foreach(Vec2, it, &s, KIND_VEC2)    { vec2_count++;    (void)it; }
    entity_foreach(Vec2, it, &s, KIND_MONSTER) { monster_count++; (void)it; }
    EXPECT(vec2_count == 1 && monster_count == 0);
}

/* ── main ────────────────────────────────────────────────────────────────── */

int main(void) {
    SECTION("Initialisation");
    RUN(reset_initialises_counts);
    RUN(reset_clears_free_list);

    SECTION("Create");
    RUN(create_returns_valid_index);
    RUN(create_increments_count);

    SECTION("Get");
    RUN(get_returns_nonnull_after_create);
    RUN(get_value_survives_write);
    RUN(get_returns_null_for_index_zero);
    RUN(get_returns_null_for_free_head);
    RUN(get_returns_null_for_kind_head);
    RUN(get_returns_null_for_out_of_bounds);

    SECTION("Get kind");
    RUN(get_kind_correct_after_create);
    RUN(get_kind_returns_minus1_for_null);
    RUN(get_kind_returns_minus1_for_oob);

    SECTION("Delete");
    RUN(delete_makes_get_return_null);
    RUN(delete_makes_get_kind_return_minus1);
    RUN(delete_slot_reused_on_next_create);
    RUN(delete_noop_on_zero);
    RUN(delete_noop_on_oob);
    RUN(delete_noop_on_already_deleted);

    SECTION("Multiple kinds");
    RUN(multiple_kinds_independent);
    RUN(multiple_kinds_foreach_independent);

    SECTION("Foreach");
    RUN(foreach_visits_all_created);
    RUN(foreach_skips_deleted);
    RUN(foreach_empty_store);
    RUN(foreach_after_delete_all);
    RUN(foreach_modifies_values_in_place);

    SECTION("Stress");
    RUN(stress_create_all);
    RUN(stress_create_delete_interleaved);
    RUN(stress_foreach_count);
    RUN(stress_reset_reuse);

    SECTION("Boundary / adversarial");
    RUN(boundary_single_kind);
    RUN(boundary_index_one_past_kinds);
    RUN(boundary_get_kind_for_free_slot);

    SECTION("Move");
    RUN(move_changes_kind);
    RUN(move_entity_still_accessible);
    RUN(move_appears_in_new_kind_foreach);
    RUN(move_absent_from_old_kind_foreach);
    RUN(move_noop_same_kind);
    RUN(move_noop_on_deleted);
    RUN(move_noop_on_oob);
    RUN(move_noop_on_zero);
    RUN(move_preserves_other_entities);
    RUN(move_back_and_forth);

end:
    printf("\n%d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}
