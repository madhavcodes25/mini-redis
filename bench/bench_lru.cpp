#include "expiring_store.hpp"
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

double run(size_t capacity, size_t distinct, size_t accesses, bool skewed) {
    ExpiringStore store(capacity);
    std::mt19937 rng(42);
    const size_t hot = distinct / 5;
    std::uniform_real_distribution<double> coin(0.0, 1.0);
    std::uniform_int_distribution<size_t> any(0, distinct - 1);
    std::uniform_int_distribution<size_t> in_hot(0, hot - 1);
    std::uniform_int_distribution<size_t> in_cold(hot, distinct - 1);

    size_t hits = 0;
    for (size_t i = 0; i < accesses; i++) {
        size_t id;
        if (!skewed)               id = any(rng);
        else if (coin(rng) < 0.8)  id = in_hot(rng);
        else                       id = in_cold(rng);

        std::string key = "key" + std::to_string(id);
        if (store.get(key)) hits++;
        else store.set(key, "v");                  // cache miss: fill it
    }
    return 100.0 * hits / accesses;
}

int main() {
    const size_t distinct = 100000, accesses = 2000000;
    std::cout << "distinct keys: " << distinct << ", accesses: " << accesses << "\n";
    std::cout << "capacity   uniform   skewed 80/20\n";
    for (double frac : {0.01, 0.05, 0.10, 0.20, 0.50}) {
        size_t cap = static_cast<size_t>(distinct * frac);
        std::cout << std::fixed << std::setprecision(1)
                  << std::setw(6) << frac * 100 << "%  "
                  << std::setw(7) << run(cap, distinct, accesses, false) << "%  "
                  << std::setw(7) << run(cap, distinct, accesses, true) << "%\n";
    }
}
