# [`nob_utils.h`](./src/nob_utils.h) - Utility plugin libraries for [nob.h](https://github.com/tsoding/nob.h)

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

See `test_` files in [`tests`](./tests/) directory to see few example usage of this library.

## Thirdparty

| Library | Reference |
| -- | -- |
| [`nob.h`](./thirdparty/nob.h) | https://github.com/tsoding/nob.h |
| [`jim.h`](./thirdparty/jim.h) | https://github.com/tsoding/jim |
| [`jimp.h`](./thirdparty/jimp.h) | https://github.com/tsoding/jim |

**Note that I have modified jimp.h for my liking**

## Current Implementations

| Library | Description | Dependencies |
| ------- | ----------- | ------------ |
| [`nob_fa.h`](./src/nob_fa.h) | It provided nob's dynamic array ops like da_append and da_pop on a fixed length array | No Dependecies |
| [`nob_heapq.h`](./src/nob_heapq.h) | It is a library that provides [`Python's heapq`](https://docs.python.org/3/library/heapq.html) | [`nob.h`](./thirdparty/nob.h) and/or [`nob_fa.h`](./src/nob_fa.h) Heapq supports both dynamic and fixed arrays |
| [`nob_deque.h`](./src/nob_deque.h) | It is a library that provides Double ended queue using [Circular Buffer](https://en.wikipedia.org/wiki/Circular_buffer) | No Dependencies |
| [`nob_fixed_deque.h`](./src/nob_fixed_deque.h) | It is a library that provides Fixed size Double ended queue using [Circular Buffer](https://en.wikipedia.org/wiki/Circular_buffer) | No Dependencies |
| [`nob_hash.h`](./src/nob_hash.h) | It is a library that provides many hash functions that can be used with hash tables | No Dependencies |
| [`nob_ht.h`](./src/nob_ht.h) | It is a library that provides hash table implementation using [Open Addressing](https://en.wikipedia.org/wiki/Open_addressing) | [`nob_hash.h`](./src/nob_hash.h) for the hash functions |
| [`nob_ilist.h`](./src/nob_ilist.h) | It is a library that provides an implementation of "Intrusive" list that was discussed in Wookash Podcast: Avoiding Modern C++ \| Anton Mikhailov (https://youtu.be/ShSGHb65f3M?si=EBeDwAQ3FkwtzqBv) | No Dependencies |
| [`nob_profiler.h`](./src/nob_profiler.h) | It is a basic profiler which was discussed by Casey Muratori on his [Computer Enhance Course](https://www.computerenhance.com/p/profiling-recursive-blocks) | [`nob.h`](./thirdparty/nob.h) for nob_log and [`nob_fa.h`](./src/nob_fa.h) for the anchors and blocks |
| [`nob_graph.h`](./src/nob_graph.h) | Simple graph library | Built on top of [`nob.h`](./thirdparty/nob.h), [`nob_deque.h`](./src/nob_deque.h) and [`nob_ht.h`](./src/nob_ht.h) |
| [`nob_rc.h`](./src/nob_rc.h) | A Simple Ref. Counting Allocator inspired from Tsoding Daily - [Reference Counting in C](https://youtu.be/iotrPxUnTdQ) | No Dependencies |
| [`nob_huge_page_alloc.h`](./src/nob_huge_page_alloc.h) | A simple Huge Page Allocator using MMAP (in Linux and Macos), and VirtualAlloc for Windows | No Dependencies |
| [`nob_br.h`](./src/nob_br.h) | A simple Buffered Reader implementation | Depends on [`nob.h`](./thirdparty/nob.h) for `String_Builder`, and `nob_log` |
| [`nob_jsonrpc.h`](./src/nob_jsonrpc.h) | A simple [jsonrpc](https://www.jsonrpc.org/specification) implementation | Depends on [`nob.h`](./thirdparty/nob.h), [`jimp.h`](./thirdparty/jimp.h), [`jim.h`](./thirdparty/jim.h) and [`nob_br.h`](./src/nob_br.h) |
| [`nob_mcp.h`](./src/nob_mcp.h) | A simple [MCP Server](https://en.wikipedia.org/wiki/Model_Context_Protocol) which implements tools/list and tools/call to be useful for creating MCP servers | Depends on [`nob.h`](./thirdparty/nob.h), [`jimp.h`](./thirdparty/jimp.h), [`jim.h`](./thirdparty/jim.h), [`nob_br.h`](./src/nob_br.h) and [`nob_jsonrpc.h`](src/nob_jsonrpc.h) |

**Note No Dependencies except libc :)**

## Copyrights

Licensed under [@MIT](./License)
