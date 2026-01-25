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

## Current Implementations

| Library | Description |
| ------- | ----------- |
| [`nob_heapq.h`](./nob_heapq.h) | It is a library that provides [`Python's heapq`](https://docs.python.org/3/library/heapq.html) |
| [`nob_deque.h`](./nob_deque.h) | It is a library that provides Double ended queue using [Circular Buffer](https://en.wikipedia.org/wiki/Circular_buffer) |
| [`nob_hash.h`](./nob_hash.h) | It is a library that provides many hash functions that can be used with hash tables |

## Copyrights

Licensed under [@MIT](./License)
