# Mini-Redis

A small in-memory key-value store written in C++17, in the spirit of Redis. It keeps string keys and values in memory, lets keys expire after a set time, drops the least recently used keys when it is full, and is driven by a command-line interface that understands a Redis-style command set.

Built for a Data Structures and Algorithms course. The goal was to implement the data structures behind a cache by hand (a hash table, a heap for expiry, a linked list for LRU) and to measure them honestly.

## Features

- **Hash table written from scratch:** separate chaining, automatic resizing when the load factor passes 0.75.
- **Key expiry (TTL):** keys can expire after a number of seconds. Expired keys are removed when they are read, and also by a periodic sweep driven by a min-heap.
- **LRU eviction:** when the store holds more keys than its capacity, the least recently used key is removed. Reads and writes both count as use.
- **Command-line interface** with Redis-style commands and replies.
- **Automated tests** that use a fake clock, so expiry is tested without waiting.
- **Benchmarks** for the hash table against `std::unordered_map`, and for LRU hit rates (see `BENCHMARKS.md`).

## Build and run

Needs a C++17 compiler and CMake 3.14 or newer. The first configure downloads Catch2 (the test framework), so it needs an internet connection and can take a minute.

```
mkdir build && cd build
cmake ..
cmake --build .
./tests              # run the automated tests
./mini_redis_cli     # start the command-line interface
./bench              # hash table against std::unordered_map
./bench_lru          # LRU hit rates
```

The build defaults to Release mode, which the benchmarks need to be meaningful.

## Commands

| Command | Reply | What it does |
|---|---|---|
| `PING` | `PONG` | Checks that the store is alive |
| `SET key value` | `OK` | Stores a value. Overwriting a key also removes its expiry |
| `GET key` | the value, or `(nil)` | Reads a value and counts as a use for LRU |
| `DEL key` | `(integer) 1` or `0` | Deletes a key. 1 if it existed |
| `EXISTS key` | `(integer) 1` or `0` | Whether a key exists |
| `INCR key` / `DECR key` | `(integer) n` | Adds or subtracts 1. A missing key counts as 0. Keeps the key's expiry |
| `EXPIRE key seconds` | `(integer) 1` or `0` | Sets an expiry. 1 if the key exists |
| `TTL key` | `(integer) n` | Seconds left, rounded up. `-1` means no expiry, `-2` means no such key |
| `PERSIST key` | `(integer) 1` or `0` | Removes an expiry. 1 if there was one |
| `DBSIZE` | `(integer) n` | Number of stored keys |
| `EXIT` | | Leaves the program |

Commands are not case-sensitive. A wrong number of arguments gives `ERR wrong number of arguments for 'GET' command`, and an unknown command gives `ERR unknown command 'FLY'`.

### Example session

```
> SET name madhav
OK
> GET name
madhav
> EXPIRE name 5
(integer) 1
> TTL name
(integer) 5
> INCR visits
(integer) 1
> INCR visits
(integer) 2
> GET missing
(nil)
> GET name            (six seconds later)
(nil)
> FLY
ERR unknown command 'FLY'
```

The command-line program is limited to 1,000 keys, so you can see eviction happen by adding more than that.

## Design

```
main.cpp            reads a line, runs it, prints the reply
  CommandHandler    parses a line, checks it, calls the store, formats the reply
    ExpiringStore   keys with expiry times and LRU eviction
      HashTable     the key-value storage (written from scratch)
      expiry map + min-heap     which keys expire, and when
      LruIndex      which keys were used least recently
```

**Hash table.** An array of buckets, each a linked list of key-value pairs. A key goes to bucket `hash(key) mod bucket_count`. When the number of keys divided by the number of buckets passes 0.75, the bucket count doubles and every key is placed again (the new position depends on the bucket count, so the old lists cannot just be copied). Doubling keeps inserts O(1) on average.

**Expiry.** Each key with an expiry has its deadline stored in a map, and the same deadline is pushed onto a min-heap ordered by time. A key is removed in two ways: **lazily**, when a read finds it past its deadline, and by a **sweep** that pops every due entry off the heap. A heap cannot remove an item from the middle, so when a key is deleted or its expiry changes, the old heap entry is left behind. The rule that keeps this safe is that **a heap entry only counts if the map still holds the same deadline for that key**. Otherwise it is stale and is ignored. Tests cover deleting, overwriting and extending an expiry.

**LRU.** A linked list ordered from most to least recently used, plus a map from each key to its place in the list. Marking a key as used moves its node to the front, and evicting removes the node at the back. Both are O(1).

**Invariants.** Two rules hold everywhere in the code: every key in the expiry map exists in the store, and the LRU index contains exactly the keys in the store. The tests exercise them, for example by checking that an evicted key leaves no expiry behind.

**Testing with time.** `ExpiringStore` takes a clock function. The tests supply a fake clock they move by hand, so a 10-second expiry is tested instantly and never flakes.

### What is built from scratch

| Part | Written by hand | Standard library used inside |
|---|---|---|
| Key-value storage | `HashTable`: buckets, chaining, resizing, lookup, delete | `std::vector`, `std::list`, and `std::hash` for the hash function |
| Expiry | The lazy and sweep logic, and the stale-entry rule | `std::priority_queue` (the heap) and `std::unordered_map` |
| LRU | The eviction logic and the index | `std::list` and `std::unordered_map` |
| Command layer | Parsing, validation and reply formatting | `std::istringstream` |

## Benchmarks

Measured on an Apple M4 in a Release build. Full tables and method are in `BENCHMARKS.md`.

- **Hash table against `std::unordered_map`** (1,000,000 keys, median of 5 runs): inserts about 9% slower and lookups about 2% slower.
- **LRU hit rates** (100,000 keys, 2,000,000 accesses): with uniform access the hit rate equals the capacity, as it must. With an 80/20 skewed workload and capacity for 10% of the keys, LRU finds 31% of its keys, about three times the uniform figure. It reaches 72% to 98% of the best possible hit rate for that workload, and falls short mainly when rarely used keys push out popular ones.

## Limitations

- Everything is in memory, so data is lost when the program exits.
- It is a command-line program only. There is no network server, so `redis-cli` cannot connect to it.
- Values are single words, so a value cannot contain spaces.
- Only string values are supported, and expiry is in whole seconds.
- The store is single-threaded.
- Capacity counts keys, not bytes, and eviction does not consider expiry times.

## Project structure

```
include/        headers: store.hpp, hash_table.hpp, expiring_store.hpp, lru_index.hpp, command_handler.hpp
src/            implementations, and main.cpp for the command-line program
tests/          Catch2 tests for each layer
bench/          bench.cpp (hash table speed) and bench_lru.cpp (LRU hit rates)
BENCHMARKS.md   results and method
CMakeLists.txt
```

## Contributions

- **Madhav Daluka**
- **Sankalp Singh Chauhan:**
- **Mridul Gupta**