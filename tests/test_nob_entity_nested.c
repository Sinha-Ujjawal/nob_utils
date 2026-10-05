#define NOB_IMPLEMENTATION
#include "nob.h"
#define NOB_ILIST_IMPLEMENTATION
#include "nob_ilist.h"
#include "nob_entity.h"

typedef struct {
    float x;
    float y;
} Vector2;

typedef enum {
    EntityKindPlayer,
    EntityKindSword,
    EntityKindPotion,
    __count_EntityKind,
} EntityKind;

static inline const char* entity_kind_as_str(EntityKind kind) {
    static_assert(__count_EntityKind == 3, "Handle missing EntityKind!");
    switch (kind) {
        case EntityKindPlayer: return "Player";
        case EntityKindSword:  return "Sword";
        case EntityKindPotion: return "Potion";
        case __count_EntityKind:
        default:
           return "Unkown";
    }
}

typedef struct {
    size_t value;
} EntityIndex;

typedef struct {
    EntityKind kind;
    EntityIndex idx;
    const char *name;
    Vector2 posn;
    union {
        size_t health; // Applicable for EntityKindPlayer and EntityKindPotion
        size_t strength; // Applicable for EntityKindSword
    };
    embed_ilist_node;
} Entity;

typedef struct {
    embed_entities(Entity);
} Entities;

static inline void log_entity(Entity entity, size_t space) {
    nob_log(INFO, "%*.sIdx: %zu", space, " ", entity.idx.value);
    nob_log(INFO, "%*.sName: %s", space, " ", entity.name);
    nob_log(INFO, "%*.sPosn: (%f, %f)", space, " ", entity.posn.x, entity.posn.y);
    nob_log(INFO, "%*.sHealth: %zu", space, " ", entity.health);
    nob_log(INFO, "%*.sStrength: %zu", space, " ", entity.strength);
}

static inline EntityIndex add_entity(Entities *entities, Entity entity) {
    EntityIndex ret = {0};
    entity_create(entities, entity.kind, &ret.value);
    entity.idx = ret;
    entities->items[ret.value].value = entity;
    return ret;
}

static inline Entity* get_entity(Entities entities, EntityIndex idx) {
    if (idx.value < entities.count) {
        return &entities.items[idx.value].value;
    }
    return NULL;
}

static inline bool delete_entity(Entities *entities, EntityIndex idx) {
    Entity *entity = get_entity(*entities, idx);
    if (entity != NULL) {
        memset(entity, 0, sizeof(Entity));
        entity_delete(entities, idx.value);
        return true;
    }
    return false;
}

static inline bool convert_kind(Entities *entities, EntityIndex idx, EntityKind newKind) {
    Entity *entity = get_entity(*entities, idx);
    if (entity != NULL) {
        entity_move(entities, idx.value, newKind);
        entity->kind = newKind;
        return true;
    }
    return false;
}

static inline bool link_sword_to_player(Entities *entities, EntityIndex playerIdx, EntityIndex swordIdx) {
    Entity *player = get_entity(*entities, playerIdx);
    if (player == NULL || player->kind != EntityKindPlayer) return false;
    Entity *sword = get_entity(*entities, swordIdx);
    if (sword == NULL || sword->kind != EntityKindSword || sword->parent != 0) return false;
#define accessor(x) (x).value
    ilist_append(entities->items, accessor, playerIdx.value, swordIdx.value);
#undef accessor
    return true;
}

static inline EntityIndex delink_sword_from_player(Entities *entities, EntityIndex swordIdx) {
    Entity *sword = get_entity(*entities, swordIdx);
    if (sword == NULL || sword->kind != EntityKindSword || sword->parent == 0) return (EntityIndex){0};
    size_t parent = sword->parent;
#define accessor(x) (x).value
    ilist_delink(entities->items, accessor, swordIdx.value);
#undef accessor
    return (EntityIndex){parent};
}

int main(void) {
    Entities entities = {0};
    entity_reset(&entities, __count_EntityKind);
    
    // Two Players
    EntityIndex player1Idx = add_entity(&entities, (Entity) {
        .kind = EntityKindPlayer,
        .name = "Ujjawal",
        .posn = (Vector2){ 10, 10 },
        .health = 100,
    });
    EntityIndex player2Idx = add_entity(&entities, (Entity) {
        .kind = EntityKindPlayer,
        .name = "Saumya",
        .posn = (Vector2){ 16, 20 },
        .health = 110,
    });

    // Three Swords
    EntityIndex masterSwordIdx = add_entity(&entities, (Entity) {
        .kind = EntityKindSword,
        .name = "Master Sword",
        .strength = INT_MAX,
    });
    EntityIndex excaliberSwordIdx = add_entity(&entities, (Entity) {
        .kind = EntityKindSword,
        .name = "Ex-Caliber Sword",
        .strength = 10000000,
    });
    EntityIndex busterSwordIdx = add_entity(&entities, (Entity) {
        .kind = EntityKindSword,
        .name = "Buster Sword",
        .strength = 1000000,
    });

    link_sword_to_player(&entities, player1Idx, masterSwordIdx);
    link_sword_to_player(&entities, player2Idx, excaliberSwordIdx);
    // link_sword_to_player(&entities, player2Idx, busterSwordIdx);

    nob_log(INFO, "All Entities:");
    for (EntityKind kind = 0; kind < __count_EntityKind; kind++) {
        entity_foreach(Entity, it, &entities, kind) {
            nob_log(INFO, "Kind: %s", entity_kind_as_str(kind));
            log_entity(*it, 2);
        }
    }

    nob_log(INFO, "Swords of each player:");
    entity_foreach(Entity, it, &entities, EntityKindPlayer) {
        nob_log(INFO, "Kind: %s", entity_kind_as_str(it->kind));
        log_entity(*it, 2);
        nob_log(INFO, "  Swords:");
#define accessor(x) (x).value
        ilist_foreach(Entity, it2, &entities.items, accessor, it->idx.value) {
            nob_log(INFO, "  Kind: %s", entity_kind_as_str(it2->kind));
            log_entity(*it2, 4);
        }
#undef accessor
    }

    nob_log(INFO, "Delinking swords from their players:");
    entity_foreach(Entity, it, &entities, EntityKindSword) {
        EntityIndex playerIdx = delink_sword_from_player(&entities, it->idx);
        if (playerIdx.value != 0) {
            Entity *player = get_entity(entities, playerIdx);
            nob_log(INFO, "Sword(Idx: %zu, Name: %s) delinked from Player(Idx: %zu, Name: %s)", it->idx, it->name, player->idx, player->name);
        }
    }

    nob_log(INFO, "Swords of each player (after delinking):");
    entity_foreach(Entity, it, &entities, EntityKindPlayer) {
        nob_log(INFO, "Kind: %s", entity_kind_as_str(it->kind));
        log_entity(*it, 2);
        nob_log(INFO, "  Swords:");
#define accessor(x) (x).value
        ilist_foreach(Entity, it2, &entities.items, accessor, it->idx.value) {
            nob_log(INFO, "  Kind: %s", entity_kind_as_str(it2->kind));
            log_entity(*it2, 4);
        }
#undef accessor
    }


    free(entities.items);
    return 0;
}
