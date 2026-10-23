# Benchmarks

Two measurements of Mini-Redis: the speed of its hash table compared with `std::unordered_map`, and how well LRU eviction keeps useful keys in a size-limited store. Everything was measured on one machine.

## Environment (both benchmarks)

| | |
|---|---|
| Machine | Apple M4 (MacBook Air), macOS |
| Compiler | AppleClang 21.0.0 |
| Build type | Release |

## 1. Hash table speed against `std::unordered_map`

Performance of the Mini-Redis hash table (separate chaining, resize at load factor above 0.75). All figures are the **median of 5 runs**, with 1,000,000 keys named `"key0"` to `"key999999"` inserted in order and the string `"v"` as every value.

### Results (median of 5 runs)

| Operation | Mini-Redis `HashTable` | `std::unordered_map` | Ratio (mine / std) |
|---|---|---|---|
| `set` 1M new keys | 152.2 ms (6.57M ops/s) | 139.7 ms (7.16M ops/s) | 1.09x time, about 9% slower |
| `get` 1M existing keys | 40.6 ms (24.61M ops/s) | 39.7 ms (25.20M ops/s) | 1.02x time, about 2% slower |

Single machine; key pattern is short sequential strings. All 1,000,000 lookups hit in both tables.

### Raw runs (ms)

| Run | `set` mine | `set` std | `get` mine | `get` std |
|---|---|---|---|---|
| 1 | 154.2 | 137.4 | 40.2 | 40.4 |
| 2 | 153.5 | 140.3 | 40.6 | 39.6 |
| 3 | 152.2 | 139.7 | 40.3 | 39.7 |
| 4 | 152.1 | 138.1 | 41.1 | 40.7 |
| 5 | 151.8 | 141.6 | 40.9 | 39.5 |
| **Median** | **152.2** | **139.7** | **40.6** | **39.7** |

### What the numbers say

- **`set` is consistently about 9% slower.** My table was slower in all 5 runs, and the ranges don't overlap (151.8 to 154.2 ms against 137.4 to 141.6 ms). The timing includes every resize, since the table starts at 16 buckets and doubles whenever the load factor passes 0.75.
- **`get` is roughly on par, slightly behind.** My table was slower in 4 of 5 runs, but the gap is only about 2% and the ranges overlap (40.2 to 41.1 ms against 39.5 to 40.7 ms).
- **Both tables use the same hash function** (`std::hash<std::string>`), so the comparison is about table structure, not hash quality.

## 2. LRU eviction hit rates

**Question:** when a store can only hold a fraction of all keys, how often does a request find its key still there?

**Setup:** 100,000 distinct keys and 2,000,000 accesses, with a fixed random seed (42), so the hit rates are the same on every run on the same machine. Every access is a `get`, and on a miss the key is stored with `set` (the way a cache is filled). The capacity is a percentage of the distinct keys. Two access patterns are compared:

- **Uniform:** every key is equally likely.
- **Skewed 80/20:** 80% of accesses go to a "hot" 20% of the keys (20,000 keys), and the other 20% of accesses are spread over the remaining 80,000 keys.

### Results

| Capacity | Uniform hit rate | Skewed 80/20 hit rate | Best possible, skewed (computed) | LRU as a share of best |
|---|---|---|---|---|
| 1% | 1.0% | 3.3% | 4.0% | 83% |
| 5% | 5.0% | 15.9% | 20.0% | 80% |
| 10% | 10.0% | 31.0% | 40.0% | 78% |
| 20% | 19.9% | 57.5% | 80.0% | 72% |
| 50% | 49.2% | 86.1% | 87.5% | 98% |

"Best possible" is the hit rate of a cache that magically knew in advance which keys are most popular and kept exactly those. For the skewed workload it is computed from the access probabilities: each hot key is requested with probability 0.8 / 20,000 and each cold key with 0.2 / 80,000, and the best cache of a given size holds the most popular keys.

### What the numbers say

- **Under uniform access, the hit rate equals the capacity.** If every key is equally likely, no eviction policy can do better than "the fraction of keys that fit". This also confirms that the eviction bookkeeping is correct. The slight shortfall at 50% (49.2%) comes from the start, when the first accesses all miss while the store fills up.
- **Under skewed access, LRU pays off a lot.** At 10% capacity it finds 31% of its keys, about three times the uniform figure, because the keys that are used often are the ones that stay.
- **LRU does not reach the best possible hit rate**, and the gap is widest around 20% capacity (57.5% against 80%). The reason is that a request for a rarely used key still enters the store and pushes out the key that was used least recently, which may be a hot key that happened not to be asked for in the last moments. This is called cache pollution. Frequency-based eviction (LFU) is designed to resist it, and is planned for June so the two can be compared on this same workload.
- **At 50% capacity LRU is almost optimal (98%)**, because there is room for all the hot keys plus many cold ones, so pollution costs little.

## Reproduce

```
mkdir -p build && cd build
cmake ..
cmake --build .
./bench        # hash table against std::unordered_map
./bench_lru    # LRU hit rates
```

`CMakeLists.txt` defaults to a Release build. Debug builds skip optimizations and give meaningless numbers.