#include <catch2/catch_test_macros.hpp>
#include "expiring_store.hpp"

namespace {
struct FakeClock {
    int64_t ms = 0;
    ExpiringStore::NowFn fn() { return [this] { return ms; }; }
};
}

TEST_CASE("key expires after its TTL") {
    FakeClock clock;
    ExpiringStore store(clock.fn());
    store.set("a", "1");
    REQUIRE(store.expire("a", 10) == true);
    clock.ms = 9999;
    REQUIRE(store.get("a") == "1");
    clock.ms = 10000;
    REQUIRE(store.get("a") == std::nullopt);
}

TEST_CASE("ttl counts down") {
    FakeClock clock;
    ExpiringStore store(clock.fn());
    store.set("a", "1");
    store.expire("a", 10);
    REQUIRE(store.ttl("a") == 10);
    clock.ms = 3000;
    REQUIRE(store.ttl("a") == 7);
}

TEST_CASE("ttl is -1 without expiry and -2 for a missing key") {
    FakeClock clock;
    ExpiringStore store(clock.fn());
    store.set("a", "1");
    REQUIRE(store.ttl("a") == -1);
    REQUIRE(store.ttl("nope") == -2);
}

TEST_CASE("expire on a missing key returns false") {
    FakeClock clock;
    ExpiringStore store(clock.fn());
    REQUIRE(store.expire("nope", 10) == false);
}

TEST_CASE("overwriting a key clears its TTL") {
    FakeClock clock;
    ExpiringStore store(clock.fn());
    store.set("a", "1");
    store.expire("a", 10);
    store.set("a", "2");
    REQUIRE(store.ttl("a") == -1);
    clock.ms = 20000;
    REQUIRE(store.get("a") == "2");
}

TEST_CASE("stale heap entry from a deleted key is ignored by sweep") {
    FakeClock clock;
    ExpiringStore store(clock.fn());
    store.set("a", "1");
    store.expire("a", 10);
    store.del("a");
    store.set("a", "new");          
    clock.ms = 20000;
    REQUIRE(store.sweep() == 0);
    REQUIRE(store.get("a") == "new");
}

TEST_CASE("extending a TTL is not undone by the old heap entry") {
    FakeClock clock;
    ExpiringStore store(clock.fn());
    store.set("a", "1");
    store.expire("a", 5);
    store.expire("a", 20);
    clock.ms = 6000;
    REQUIRE(store.sweep() == 0);     
    REQUIRE(store.get("a") == "1");
    clock.ms = 20000;
    REQUIRE(store.sweep() == 1);
}

TEST_CASE("sweep removes expired keys without any reads") {
    FakeClock clock;
    ExpiringStore store(clock.fn());
    for (int i = 0; i < 5; i++) {
        store.set("k" + std::to_string(i), "v");
        store.expire("k" + std::to_string(i), 5);
    }
    store.set("keep", "v");
    clock.ms = 5000;
    REQUIRE(store.size() == 6);     
    REQUIRE(store.sweep() == 5);
    REQUIRE(store.size() == 1);
}

TEST_CASE("persist removes the TTL") {
    FakeClock clock;
    ExpiringStore store(clock.fn());
    store.set("a", "1");
    store.expire("a", 10);
    REQUIRE(store.persist("a") == true);
    REQUIRE(store.ttl("a") == -1);
    clock.ms = 20000;
    REQUIRE(store.sweep() == 0);
    REQUIRE(store.get("a") == "1");
    REQUIRE(store.persist("a") == false);  
}