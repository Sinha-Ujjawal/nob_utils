# `nob_utils.h` - Utility plugin libraries for [nob.h](https://github.com/tsoding/nob.h)

This repository contains libraries build on top of or in style of [nob.h](https://github.com/tsoding/nob.h)

# Usage

```c
#include "nob.h"
#define NOB_HASH_IMPLEMENTATION // For hash function implementations
#include "nob_utils.h"

int main(void) {
    return 0;
}
```

See `test_` files to see few example usage of this library.

## Current Implementations

| Library | Description |
| ------- | ----------- |
| [`nob_fa.h`](./nob_fa.h) | It provided nob's dynamic array ops like da_append and da_pop on a fixed length array |
| [`nob_heapq.h`](./nob_heapq.h) | It is a library that provides [`Python's heapq`](https://docs.python.org/3/library/heapq.html) |
| [`nob_deque.h`](./nob_deque.h) | It is a library that provides Double ended queue using [Circular Buffer](https://en.wikipedia.org/wiki/Circular_buffer) |
| [`nob_hash.h`](./nob_hash.h) | It is a library that provides many hash functions that can be used with hash tables |
| [`nob_ht.h`](./nob_ht.h) | It is a library that provides hash table implementation using [Open Addressing](https://en.wikipedia.org/wiki/Open_addressing) |
| [`nob_ilist.h`](./nob_ilist.h) | It is a library that provides an implementation of "Intrusive" list that was discussed in Wookash Podcast: Avoiding Modern C++ | Anton Mikhailov (https://youtu.be/ShSGHb65f3M?si=EBeDwAQ3FkwtzqBv) |
| [`nob_profiler.h`](./nob_profiler.h) | It is a basic profiler which was discussed by Casey Muratori on his [Computer Enhance Course](https://www.computerenhance.com/p/profiling-recursive-blocks) |

## Copyrights

Licensed under [@MIT](./License)
