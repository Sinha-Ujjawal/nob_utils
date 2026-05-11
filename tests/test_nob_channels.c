#define NOB_IMPLEMENTATION
#define NOB_BR_IMPLEMENTATION
#include "nob.h"
#include "nob_fixed_deque.h"
#include "nob_channels.h"

#include <stdio.h>
#include <unistd.h>
#include <stdatomic.h>
#include <assert.h>

// ============================================================
// Helpers
// ============================================================

#define TEST(name) do { fprintf(stderr, "[ RUN ] %s\n", name); } while(0)
#define PASS(name) do { fprintf(stderr, "[ OK  ] %s\n", name); } while(0)
#define FAIL(name, msg) do { fprintf(stderr, "[FAIL ] %s: %s\n", name, msg); abort(); } while(0)

typedef embed_channel(int, 4)  Int_Chan;
typedef embed_channel(int, 0)  Int_Chan0;  // unbuffered

// ============================================================
// Test 1: Basic buffered send/recv (single thread)
// ============================================================
void test_basic_buffered(void) {
    const char *T = "basic_buffered";
    TEST(T);
    Int_Chan ch;
    channel_init(&ch);

    bool ok;
    int val;

    channel_send(&ch, 10);
    channel_send(&ch, 20);
    channel_send(&ch, 30);

    channel_recv(&ch, &val, &ok);
    assert(ok && val == 10);
    channel_recv(&ch, &val, &ok);
    assert(ok && val == 20);
    channel_recv(&ch, &val, &ok);
    assert(ok && val == 30);

    PASS(T);
}

// ============================================================
// Test 2: Close — recv on empty closed channel returns ok=false
// ============================================================
void test_close_empty(void) {
    const char *T = "close_empty";
    TEST(T);
    Int_Chan ch;
    channel_init(&ch);

    channel_send(&ch, 1);
    channel_close(&ch);

    bool ok;
    int val;

    // Buffered item still readable after close
    channel_recv(&ch, &val, &ok);
    assert(ok && val == 1);

    // Now empty + closed
    channel_recv(&ch, &val, &ok);
    assert(!ok);

    PASS(T);
}

// ============================================================
// Test 3: Blocking send (buffer full) — producer blocks until consumer drains
// ============================================================
typedef struct { Int_Chan *ch; atomic_int *order; } T3_Args;

void *t3_consumer(void *arg) {
    T3_Args *a = arg;
    usleep(20000); // let producer fill and block
    bool ok; int val;
    for (int i = 0; i < 6; i++) {
        channel_recv(a->ch, &val, &ok);
        assert(ok && val == i);
    }
    return NULL;
}

void test_blocking_send(void) {
    const char *T = "blocking_send";
    TEST(T);
    Int_Chan ch;
    channel_init(&ch);
    atomic_int order = 0;
    T3_Args args = { &ch, &order };

    pthread_t consumer;
    pthread_create(&consumer, NULL, t3_consumer, &args);

    for (int i = 0; i < 6; i++) channel_send(&ch, i);

    pthread_join(consumer, NULL);
    PASS(T);
}

// ============================================================
// Test 4: Blocking recv — consumer blocks until producer sends
// ============================================================
typedef struct { Int_Chan *ch; } T4_Args;

void *t4_producer(void *arg) {
    T4_Args *a = arg;
    usleep(20000);
    for (int i = 0; i < 4; i++) channel_send(a->ch, i);
    channel_close(a->ch);
    return NULL;
}

void test_blocking_recv(void) {
    const char *T = "blocking_recv";
    TEST(T);
    Int_Chan ch;
    channel_init(&ch);
    T4_Args args = { &ch };

    pthread_t producer;
    pthread_create(&producer, NULL, t4_producer, &args);

    bool ok; int val; int count = 0;
    while (true) {
        channel_recv(&ch, &val, &ok);
        if (!ok) break;
        assert(val == count++);
    }
    assert(count == 4);

    pthread_join(producer, NULL);
    PASS(T);
}

// ============================================================
// Test 5: Multiple producers + multiple consumers, count items
// ============================================================
#define T5_PRODUCERS 8
#define T5_CONSUMERS 8
#define T5_ITEMS_PER_PRODUCER 100

typedef struct { Int_Chan *ch; int id; } T5_PArgs;
typedef struct { Int_Chan *ch; atomic_int *total; } T5_CArgs;

void *t5_producer(void *arg) {
    T5_PArgs *a = arg;
    for (int i = 0; i < T5_ITEMS_PER_PRODUCER; i++) channel_send(a->ch, 1);
    return NULL;
}
void *t5_consumer(void *arg) {
    T5_CArgs *a = arg;
    bool ok; int val;
    while (true) {
        channel_recv(a->ch, &val, &ok);
        if (!ok) break;
        atomic_fetch_add(a->total, 1);
    }
    return NULL;
}

void test_concurrent(void) {
    const char *T = "concurrent_producers_consumers";
    TEST(T);
    Int_Chan ch;
    channel_init(&ch);
    atomic_int total = 0;

    pthread_t producers[T5_PRODUCERS], consumers[T5_CONSUMERS];
    T5_PArgs pargs[T5_PRODUCERS];
    T5_CArgs cargs = { &ch, &total };

    for (int i = 0; i < T5_CONSUMERS; i++)
        pthread_create(&consumers[i], NULL, t5_consumer, &cargs);
    for (int i = 0; i < T5_PRODUCERS; i++) {
        pargs[i] = (T5_PArgs){ &ch, i };
        pthread_create(&producers[i], NULL, t5_producer, &pargs[i]);
    }
    for (int i = 0; i < T5_PRODUCERS; i++) pthread_join(producers[i], NULL);
    channel_close(&ch);
    for (int i = 0; i < T5_CONSUMERS; i++) pthread_join(consumers[i], NULL);

    assert(atomic_load(&total) == T5_PRODUCERS * T5_ITEMS_PER_PRODUCER);
    PASS(T);
}

// ============================================================
// Test 6: channel_alt — fast path, both channels have data
// ============================================================
void test_alt_fast_path(void) {
    const char *T = "alt_fast_path";
    TEST(T);
    Int_Chan a, b;
    channel_init(&a);
    channel_init(&b);

    channel_send(&a, 111);
    channel_send(&b, 222);

    // First alt should pick one of them (whichever is first in arm order)
    int va = -1, vb = -1;
    bool ok_a, ok_b;
    int which;

    #define ARMS1(Send, Recv) Recv(&a, &va, &ok_a) Recv(&b, &vb, &ok_b)
    channel_alt(which, ARMS1);
    #undef ARMS1
    assert(which == 0 || which == 1);
    // The winning channel's value should be set
    if (which == 0) assert(ok_a && va == 111);
    else            assert(ok_b && vb == 222);

    PASS(T);
}

// ============================================================
// Test 7: channel_alt — slow path, recv arm woken by sender
// ============================================================
typedef struct { Int_Chan *ch; int val; } T7_Args;

void *t7_sender(void *arg) {
    T7_Args *a = arg;
    usleep(20000);
    channel_send(a->ch, a->val);
    return NULL;
}

void test_alt_slow_path_recv(void) {
    const char *T = "alt_slow_path_recv";
    TEST(T);
    Int_Chan a, b;
    channel_init(&a);
    channel_init(&b);

    // Only chan_b will receive a value
    T7_Args args = { &b, 42 };
    pthread_t sender;
    pthread_create(&sender, NULL, t7_sender, &args);

    int va = -1, vb = -1;
    bool ok_a, ok_b;
    int which;

    #define ARMS2(Send, Recv) Recv(&a, &va, &ok_a) Recv(&b, &vb, &ok_b)
    channel_alt(which, ARMS2);
    #undef ARMS2

    assert(which == 1);
    assert(ok_b && vb == 42);

    pthread_join(sender, NULL);
    PASS(T);
}

// ============================================================
// Test 8: channel_alt — send arm fires when receiver is waiting
// ============================================================
typedef struct { Int_Chan *ch; int *got; bool *ok; } T8_Args;

void *t8_receiver(void *arg) {
    T8_Args *a = arg;
    usleep(20000);
    channel_recv(a->ch, a->got, a->ok);
    return NULL;
}

void test_alt_send_arm(void) {
    const char *T = "alt_send_arm";
    TEST(T);
    Int_Chan a, b;
    channel_init(&a);
    channel_init(&b);

    int got = -1; bool ok = false;
    T8_Args rargs = { &b, &got, &ok };
    pthread_t receiver;
    pthread_create(&receiver, NULL, t8_receiver, &rargs);

    usleep(40000); // ensure receiver is blocked

    int dummy = -1; bool ok_a;
    int which;

    // Send arm on b; recv arm on a (a has nothing)
    #define ARMS3(Send, Recv) Recv(&a, &dummy, &ok_a) Send(&b, 99)
    channel_alt(which, ARMS3);
    #undef ARMS3

    assert(which == 1); // Send arm fired

    pthread_join(receiver, NULL);
    assert(ok && got == 99);

    PASS(T);
}

// ============================================================
// Test 9: channel_alt called twice in the same function (label safety)
// ============================================================
void test_alt_twice_same_function(void) {
    const char *T = "alt_twice_same_function";
    TEST(T);
    Int_Chan a, b;
    channel_init(&a);
    channel_init(&b);

    channel_send(&a, 1);
    channel_send(&b, 2);

    int va = -1, vb = -1;
    bool ok_a, ok_b;
    int which;

    #define FIRST_ARMS(Send, Recv) Recv(&a, &va, &ok_a) Recv(&b, &vb, &ok_b)
    channel_alt(which, FIRST_ARMS);
    #undef FIRST_ARMS
    assert(which == 0 && va == 1);

    va = -1; vb = -1;
    #define SECOND_ARMS(Send, Recv) Recv(&a, &va, &ok_a) Recv(&b, &vb, &ok_b)
    channel_alt(which, SECOND_ARMS);
    #undef SECOND_ARMS
    assert(which == 1 && vb == 2);

    PASS(T);
}

// ============================================================
// Test 10: alt on closed channel — fires immediately with ok=false
// ============================================================
void test_alt_closed(void) {
    const char *T = "alt_closed";
    TEST(T);
    Int_Chan a, b;
    channel_init(&a);
    channel_init(&b);
    channel_close(&b);

    int va = -1, vb = -1;
    bool ok_a = true, ok_b = true;
    int which;

    #define ARMS4(Send, Recv) Recv(&a, &va, &ok_a) Recv(&b, &vb, &ok_b)
    channel_alt(which, ARMS4);
    #undef ARMS4

    assert(which == 1);
    assert(!ok_b);

    PASS(T);
}

// ============================================================
// Main
// ============================================================
int main(void) {
    test_basic_buffered();
    test_close_empty();
    test_blocking_send();
    test_blocking_recv();
    test_concurrent();
    test_alt_fast_path();
    test_alt_slow_path_recv();
    test_alt_send_arm();
    test_alt_twice_same_function();
    test_alt_closed();

    fprintf(stderr, "\nAll tests passed.\n");
    return 0;
}
