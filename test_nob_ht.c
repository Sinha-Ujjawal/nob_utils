#include <stdio.h>
#include <string.h>

#define NOB_IMPLEMENTATION
#include "nob.h"

#define NOB_HASH_IMPLEMENTATION
#define NOB_HT_INIT_CAP 8
#include "nob_utils.h"

#define is_cstr_equal(x, y) (strcmp(x, y) == 0)

typedef struct {
    const char *key;
    size_t value;
    bool is_occupied;
    bool is_not_new;
} My_KV_Slot;

typedef struct {
    My_KV_Slot *items;
    size_t count;
    size_t capacity;
} My_Hash_Table;

int main(void) {
    My_Hash_Table ht = {0};
    // ht_reserve(&ht, hash_cstr, is_cstr_equal, HT_INIT_CAP);
    int key_idx;
    ht_insert_key(&ht, hash_cstr, is_cstr_equal, "Hello", &key_idx);
    ht.items[key_idx].value = 69;
    if (!ht.items[key_idx].is_not_new) {
        printf("Key `%s` is new!\n", ht.items[key_idx].key);
        ht.items[key_idx].is_not_new = true;
    } else {
        printf("Key `%s` is not new!\n", ht.items[key_idx].key);
    }
    ht_foreach(My_KV_Slot, slot, &ht) {
        printf("Key: %s\n", slot->key);
        printf("  Value: %zu\n", slot->value);
        printf("  Is Occupied: %s\n", slot->is_occupied ? "true" : "false");
        printf("  Is Not New: %s\n", slot->is_not_new ? "true" : "false");
    }
    ht_insert_key(&ht, hash_cstr, is_cstr_equal, "Hello", &key_idx);
    if (!ht.items[key_idx].is_not_new) {
        printf("Key `%s` is new!\n", ht.items[key_idx].key);
        ht.items[key_idx].is_not_new = true;
    } else {
        printf("Key `%s` is not new!\n", ht.items[key_idx].key);
    }
    ht.items[key_idx].value = 72;
    printf("My hash table contents (%zu items):\n", ht.count);
    ht_foreach(My_KV_Slot, slot, &ht) {
        printf("Key: %s\n", slot->key);
        printf("  Value: %zu\n", slot->value);
        printf("  Is Occupied: %s\n", slot->is_occupied ? "true" : "false");
        printf("  Is Not New: %s\n", slot->is_not_new ? "true" : "false");
    }
    size_t mark = temp_save();
    printf("Adding 16 keys to the hash table\n");
    for (size_t i = 0; i < 16; i++) {
        char *key = temp_sprintf("Hello %zu", i);
        ht_insert_key(&ht, hash_cstr, is_cstr_equal, key, &key_idx);
        ht.items[key_idx].value = i * 20;
    }
    printf("My hash table contents (%zu items):\n", ht.count);
    ht_foreach(My_KV_Slot, slot, &ht) {
        printf("Key: %s\n", slot->key);
        printf("  Value: %zu\n", slot->value);
        printf("  Is Occupied: %s\n", slot->is_occupied ? "true" : "false");
        printf("  Is Not New: %s\n", slot->is_not_new ? "true" : "false");
    }
    temp_rewind(mark);

    My_KV_Slot slot;
    ht_delete_key(&ht, hash_cstr, is_cstr_equal, "Hello", &slot);
    printf("Deleted Key: %s\n", slot.key);
    printf("  Value: %zu\n", slot.value);
    printf("  Is Occupied: %s\n", slot.is_occupied ? "true" : "false");
    printf("  Is Not New: %s\n", slot.is_not_new ? "true" : "false");

    printf("My hash table contents (%zu items):\n", ht.count);
    ht_foreach(My_KV_Slot, slot, &ht) {
        printf("Key: %s\n", slot->key);
        printf("  Value: %zu\n", slot->value);
        printf("  Is Occupied: %s\n", slot->is_occupied ? "true" : "false");
        printf("  Is Not New: %s\n", slot->is_not_new ? "true" : "false");
    }

    free(ht.items);
    return 0;
}
