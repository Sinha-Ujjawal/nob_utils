#include <stdio.h>

#define NOB_IMPLEMENTATION
#include "nob.h"

#define NOB_HASH_IMPLEMENTATION
#include "nob_utils.h"

#define hash_uint_69(x) mix_hash(hash_uint(x), 69)

typedef struct {
    const char *name;
    size_t age;
} Person;

#define hash_person(p)       \
    combine_hash(            \
        HASH_DEFAULT_MIX,    \
        2,                   \
        hash_cstr((p).name), \
        hash_uint((p).age),  \
    )

int main(void) {
    printf("Testing hash_uint\n");
    for (size_t i = 0; i < 100; i++) {
        printf("hash_uint(%zu) => %zu\n", i, hash_uint(i));
    }
    printf("Testing hash_int\n");
    for (int i = -100; i < 100; i++) {
        printf("hash_int(%d) => %zu\n", i, hash_int(i));
    }
    printf("Testing hash_uint_69\n");
    for (size_t i = 0; i < 100; i++) {
        printf("hash_uint_69(%zu) => %zu\n", i, hash_uint_69(i));
    }
    printf("Testing hash_person\n");
    size_t mark = temp_save();
    for (size_t i = 0; i < 10; i++) {
        Person person = {
            .name = temp_sprintf("Person %zu", i),
            .age = i * 20,
        };
        printf("hash_person(Person{name: %s, age: %zu}) => %zu\n", person.name, person.age, hash_person(person));
    }
    for (size_t i = 0; i < 10; i++) {
        Person person = {
            .name = temp_sprintf("Person %zu", i),
            .age = i * 20,
        };
        printf("hash_person(Person{name: %s, age: %zu}) => %zu\n", person.name, person.age, hash_person(person));
    }
    temp_rewind(mark);
    return 0;
}
