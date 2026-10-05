# Benchmarks

Performance of the Mini-Redis hash table (separate chaining, resize at load factor > 0.75) compared with `std::unordered_map`. All figures below are the **median of 5 runs**.

## Environment

| | |
|---|---|
| Machine | Apple M4 (MacBook Air), macOS |
| Compiler | AppleClang 21.0.0 |
| Build type | Release |
| Workload size | N = 1,000,000 keys |
| Keys | `"key0"` to `"key999999"`, inserted in order |
| Values | the string `"v"` |

## Results (median of 5 runs)

| Operation | Mini-Redis `HashTable` | `std::unordered_map` | Ratio (mine / std) |
|---|---|---|---|
| `set` 1M new keys | 152.2 ms (6.57M ops/s) | 139.7 ms (7.16M ops/s) | 1.09x time, about 9% slower |
| `get` 1M existing keys | 40.6 ms (24.61M ops/s) | 39.7 ms (25.20M ops/s) | 1.02x time, about 2% slower |

Single machine; key pattern is short sequential strings. All 1,000,000 lookups hit in both tables.

## Raw runs (ms)

| Run | `set` mine | `set` std | `get` mine | `get` std |
|---|---|---|---|---|
| 1 | 154.2 | 137.4 | 40.2 | 40.4 |
| 2 | 153.5 | 140.3 | 40.6 | 39.6 |
| 3 | 152.2 | 139.7 | 40.3 | 39.7 |
| 4 | 152.1 | 138.1 | 41.1 | 40.7 |
| 5 | 151.8 | 141.6 | 40.9 | 39.5 |
| **Median** | **152.2** | **139.7** | **40.6** | **39.7** |

## What the numbers say

- **`set` is consistently about 9% slower.** My table was slower in all 5 runs, and the ranges don't overlap (151.8 to 154.2 ms against 137.4 to 141.6 ms). The timing includes every resize, since the table starts at 16 buckets and doubles whenever the load factor passes 0.75.
- **`get` is roughly on par, slightly behind.** My table was slower in 4 of 5 runs, but the gap is only about 2% and the ranges overlap (40.2 to 41.1 ms against 39.5 to 40.7 ms).
- **Both tables use the same hash function** (`std::hash<std::string>`), so the comparison is about table structure, not hash quality.

## Reproduce

```
mkdir -p build && cd build
cmake ..
cmake --build .
./bench
```

`CMakeLists.txt` defaults to a Release build. Debug builds skip optimizations and give meaningless numbers.