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

| Library                         | Reference                        |
| ---                             | ---                              |
| [`nob.h`](./thirdparty/nob.h)   | https://github.com/tsoding/nob.h |
| [`jim.h`](./thirdparty/jim.h)   | https://github.com/tsoding/jim   |
| [`jimp.h`](./thirdparty/jimp.h) | https://github.com/tsoding/jim   |

**Note that I have modified jimp.h for my liking**

## Current Implementations

| Library                                                | Dependencies                                                                                                                                      | Description                                                                                                                                                                                                                                                              |
| -------                                                | ------------                                                                                                                                      | -----------                                                                                                                                                                                                                                                              |
| [`nob_ext.h`](./src/nob_ext.h)                         | [`nob.h`](./thirdparty/nob.h)                                                                                                                     | Useful extensions for [`nob.h`](./thirdparty/nob.h)                                                                                                                                                                                                                      |
| [`nob_fa.h`](./src/nob_fa.h)                           | No Dependecies                                                                                                                                    | It provided nob's dynamic array ops like da_append and da_pop on a fixed length array                                                                                                                                                                                    |
| [`nob_heapq.h`](./src/nob_heapq.h)                     | [`nob.h`](./thirdparty/nob.h) and/or [`nob_fa.h`](./src/nob_fa.h) Heapq supports both dynamic and fixed arrays                                    | It is a library that provides [`Python's heapq`](https://docs.python.org/3/library/heapq.html)                                                                                                                                                                           |
| [`nob_deque.h`](./src/nob_deque.h)                     | No Dependencies                                                                                                                                   | It is a library that provides Double ended queue using [Circular Buffer](https://en.wikipedia.org/wiki/Circular_buffer)                                                                                                                                                  |
| [`nob_fixed_deque.h`](./src/nob_fixed_deque.h)         | No Dependencies                                                                                                                                   | It is a library that provides Fixed size Double ended queue using [Circular Buffer](https://en.wikipedia.org/wiki/Circular_buffer)                                                                                                                                       |
| [`nob_hash.h`](./src/nob_hash.h)                       | No Dependencies                                                                                                                                   | It is a library that provides many hash functions that can be used with hash tables                                                                                                                                                                                      |
| [`nob_ht.h`](./src/nob_ht.h)                           | [`nob_hash.h`](./src/nob_hash.h) for the hash functions                                                                                           | It is a library that provides hash table implementation using [Open Addressing](https://en.wikipedia.org/wiki/Open_addressing)                                                                                                                                           |
| [`nob_ilist.h`](./src/nob_ilist.h)                     | No Dependencies                                                                                                                                   | It is a library that provides an implementation of "Intrusive" list that was discussed in Wookash Podcast: Avoiding Modern C++ - Anton Mikhailov (https://youtu.be/ShSGHb65f3M?si=EBeDwAQ3FkwtzqBv)                                                                      |
| [`nob_entity.h`](./src/nob_entity.h)                   | Depends on [`nob.h`](./thirdparty/nob.h) for the dynamic array, and [`nob_ilist.h`](./src/nob_ilist.h) for the Intrusive list impl.               | It is a library that provides a simple entity management system based on "Intrusive" list and Dynamic arrays                                                                                                                                                             |
| [`nob_profiler.h`](./src/nob_profiler.h)               | [`nob.h`](./thirdparty/nob.h) for nob_log and [`nob_fa.h`](./src/nob_fa.h) for the anchors and blocks                                             | It is a basic profiler which was discussed by Casey Muratori on his [Computer Enhance Course](https://www.computerenhance.com/p/profiling-recursive-blocks)                                                                                                              |
| [`nob_graph.h`](./src/nob_graph.h)                     | Built on top of [`nob.h`](./thirdparty/nob.h), [`nob_deque.h`](./src/nob_deque.h) and [`nob_ht.h`](./src/nob_ht.h)                                | Simple graph library                                                                                                                                                                                                                                                     |
| [`nob_rc.h`](./src/nob_rc.h)                           | No Dependencies                                                                                                                                   | A Simple Ref. Counting Allocator inspired from Tsoding Daily - [Reference Counting in C](https://youtu.be/iotrPxUnTdQ)                                                                                                                                                   |
| [`nob_huge_page_alloc.h`](./src/nob_huge_page_alloc.h) | No Dependencies                                                                                                                                   | A simple Huge Page Allocator using MMAP (in Linux and Macos), and VirtualAlloc for Windows                                                                                                                                                                               |
| [`nob_br.h`](./src/nob_br.h)                           | Depends on [`nob.h`](./thirdparty/nob.h) for `String_Builder`, and `nob_log`                                                                      | A simple Buffered Reader implementation                                                                                                                                                                                                                                  |
| [`nob_channels.h`](./src/nob_channels.h)               | Depends in [`nob_fixed_deque.h`](./src/nob_fixed_deque.h) for the Fixed deque impl.                                                               | A simple Channel implementation in C based on [Rich Hickey's "Inside core.async Channels" by Rich Hickey (2014) talk](https://youtu.be/hMEX6lfBeRM?si=SP45SY3rooIaru3l). Note that this is mostly for POSIX system and is mostly vibe coded using Gemini and Claude Code |
| [`nob_jsonrpc.h`](./src/nob_jsonrpc.h)                 | Depends on [`nob.h`](./thirdparty/nob.h), [`jimp.h`](./thirdparty/jimp.h) and [`jim.h`](./thirdparty/jim.h)                                       | A simple [jsonrpc](https://www.jsonrpc.org/specification) implementation                                                                                                                                                                                                 |
| [`nob_mcp.h`](./src/nob_mcp.h)                         | Depends on [`nob.h`](./thirdparty/nob.h), [`jimp.h`](./thirdparty/jimp.h), [`jim.h`](./thirdparty/jim.h) and [`nob_jsonrpc.h`](src/nob_jsonrpc.h) | A simple [MCP Server](https://en.wikipedia.org/wiki/Model_Context_Protocol) which implements tools/list and tools/call to be useful for creating MCP servers                                                                                                             |

## Supported Platforms:

1. Linux x86-64

## Copyrights

Licensed under [@MIT](./License)
