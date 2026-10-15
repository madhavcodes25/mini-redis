#include <catch2/catch_test_macros.hpp>
#include "expiring_store.hpp"
#include "lru_index.hpp"

namespace {
struct FakeClock {
    int64_t ms = 0;
    ExpiringStore::NowFn fn() { return [this] { return ms; }; }
};
}

TEST_CASE("LruIndex evicts in least-recently-used order") {
    LruIndex lru;
    lru.touch("a"); lru.touch("b"); lru.touch("c");
    lru.touch("a");                             
    REQUIRE(lru.evict() == "b");
    REQUIRE(lru.evict() == "c");
    REQUIRE(lru.evict() == "a");
    REQUIRE(lru.evict() == std::nullopt);
}

TEST_CASE("LruIndex remove drops a key") {
    LruIndex lru;
    lru.touch("a"); lru.touch("b");
    lru.remove("a");
    REQUIRE(lru.size() == 1);
    REQUIRE(lru.evict() == "b");
}

TEST_CASE("store evicts the least recently used key when over capacity") {
    FakeClock clock;
    ExpiringStore store(clock.fn(), 2);
    store.set("a", "1");
    store.set("b", "2");
    store.set("c", "3");
    REQUIRE(store.size() == 2);
    REQUIRE(store.get("a") == std::nullopt);
    REQUIRE(store.get("b") == "2");
    REQUIRE(store.get("c") == "3");
}

TEST_CASE("reading a key refreshes its recency") {
    FakeClock clock;
    ExpiringStore store(clock.fn(), 2);
    store.set("a", "1");
    store.set("b", "2");
    store.get("a");
    store.set("c", "3");
    REQUIRE(store.get("b") == std::nullopt);
    REQUIRE(store.get("a") == "1");
    REQUIRE(store.get("c") == "3");
}

TEST_CASE("overwriting a key refreshes its recency") {
    FakeClock clock;
    ExpiringStore store(clock.fn(), 2);
    store.set("a", "1");
    store.set("b", "2");
    store.set("a", "9");
    store.set("c", "3");
    REQUIRE(store.get("b") == std::nullopt);
    REQUIRE(store.get("a") == "9");
}

TEST_CASE("an expired key frees its slot instead of causing an eviction") {
    FakeClock clock;
    ExpiringStore store(clock.fn(), 2);
    store.set("a", "1");
    store.expire("a", 5);
    store.set("b", "2");
    clock.ms = 5000;
    REQUIRE(store.get("a") == std::nullopt);    
    store.set("c", "3");
    REQUIRE(store.size() == 2);
    REQUIRE(store.get("b") == "2");
    REQUIRE(store.get("c") == "3");
}

TEST_CASE("an evicted key leaves no TTL behind") {
    FakeClock clock;
    ExpiringStore store(clock.fn(), 1);
    store.set("a", "1");
    store.expire("a", 10);
    store.set("b", "2");                       
    clock.ms = 20000;
    REQUIRE(store.sweep() == 0);                
    REQUIRE(store.get("b") == "2");
}