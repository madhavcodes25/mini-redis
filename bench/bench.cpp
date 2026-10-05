#include "hash_table.hpp"
#include <chrono>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using Clock = std::chrono::steady_clock;

template <typename F>
double time_ms(F&& f) {
    auto start = Clock::now();
    f();
    return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
}

int main() {
    const int N = 1000000;
    std::vector<std::string> keys;
    keys.reserve(N);
    for (int i = 0; i < N; i++) keys.push_back("key" + std::to_string(i));

    HashTable mine;
    std::unordered_map<std::string, std::string> ref;
    size_t hits_mine = 0, hits_ref = 0;

    double mine_set = time_ms([&] { for (int i = 0; i < N; i++) mine.set(keys[i], "v"); });
    double ref_set  = time_ms([&] { for (int i = 0; i < N; i++) ref[keys[i]] = "v"; });
    double mine_get = time_ms([&] { for (int i = 0; i < N; i++) if (mine.get(keys[i])) hits_mine++; });
    double ref_get  = time_ms([&] { for (int i = 0; i < N; i++) if (ref.find(keys[i]) != ref.end()) hits_ref++; });

    auto ops = [&](double ms) { return static_cast<long long>(N / (ms / 1000.0)); };
    std::cout << "N = " << N << " (hits: " << hits_mine << ", " << hits_ref << ")\n";
    std::cout << "set  mine: " << mine_set << " ms (" << ops(mine_set) << " ops/s)\n";
    std::cout << "set  std : " << ref_set  << " ms (" << ops(ref_set)  << " ops/s)\n";
    std::cout << "get  mine: " << mine_get << " ms (" << ops(mine_get) << " ops/s)\n";
    std::cout << "get  std : " << ref_get  << " ms (" << ops(ref_get)  << " ops/s)\n";
}