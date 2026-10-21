#include <catch2/catch_test_macros.hpp>
#include "command_handler.hpp"

namespace {
struct FakeClock {
    int64_t ms = 0;
    ExpiringStore::NowFn fn() { return [this] { return ms; }; }
};
}

TEST_CASE("SET and GET") {
    FakeClock clock;
    ExpiringStore store(clock.fn());
    CommandHandler h(store);
    REQUIRE(h.execute("SET name madhav") == "OK");
    REQUIRE(h.execute("GET name") == "madhav");
    REQUIRE(h.execute("GET missing") == "(nil)");
}

TEST_CASE("commands are case-insensitive and extra spaces are ignored") {
    FakeClock clock;
    ExpiringStore store(clock.fn());
    CommandHandler h(store);
    REQUIRE(h.execute("  set   a   1 ") == "OK");
    REQUIRE(h.execute("gEt a") == "1");
}

TEST_CASE("DEL and EXISTS") {
    FakeClock clock;
    ExpiringStore store(clock.fn());
    CommandHandler h(store);
    h.execute("SET a 1");
    REQUIRE(h.execute("EXISTS a") == "(integer) 1");
    REQUIRE(h.execute("DEL a") == "(integer) 1");
    REQUIRE(h.execute("DEL a") == "(integer) 0");
    REQUIRE(h.execute("EXISTS a") == "(integer) 0");
}

TEST_CASE("INCR and DECR") {
    FakeClock clock;
    ExpiringStore store(clock.fn());
    CommandHandler h(store);
    REQUIRE(h.execute("INCR counter") == "(integer) 1");
    REQUIRE(h.execute("INCR counter") == "(integer) 2");
    REQUIRE(h.execute("DECR counter") == "(integer) 1");
    h.execute("SET word hello");
    REQUIRE(h.execute("INCR word") == "ERR value is not an integer or out of range");
}

TEST_CASE("INCR keeps the TTL") {
    FakeClock clock;
    ExpiringStore store(clock.fn());
    CommandHandler h(store);
    h.execute("SET c 5");
    h.execute("EXPIRE c 10");
    REQUIRE(h.execute("INCR c") == "(integer) 6");
    REQUIRE(h.execute("TTL c") == "(integer) 10");
    clock.ms = 10000;
    REQUIRE(h.execute("GET c") == "(nil)");
}

TEST_CASE("EXPIRE, TTL and PERSIST") {
    FakeClock clock;
    ExpiringStore store(clock.fn());
    CommandHandler h(store);
    h.execute("SET a 1");
    REQUIRE(h.execute("EXPIRE a 10") == "(integer) 1");
    REQUIRE(h.execute("TTL a") == "(integer) 10");
    clock.ms = 4000;
    REQUIRE(h.execute("TTL a") == "(integer) 6");
    REQUIRE(h.execute("PERSIST a") == "(integer) 1");
    REQUIRE(h.execute("TTL a") == "(integer) -1");
    REQUIRE(h.execute("EXPIRE nope 10") == "(integer) 0");
    REQUIRE(h.execute("TTL nope") == "(integer) -2");
}

TEST_CASE("wrong argument counts and unknown commands") {
    FakeClock clock;
    ExpiringStore store(clock.fn());
    CommandHandler h(store);
    REQUIRE(h.execute("GET") == "ERR wrong number of arguments for 'GET' command");
    REQUIRE(h.execute("SET a") == "ERR wrong number of arguments for 'SET' command");
    REQUIRE(h.execute("FLY a") == "ERR unknown command 'FLY'");
}

TEST_CASE("an empty line gives an empty reply") {
    FakeClock clock;
    ExpiringStore store(clock.fn());
    CommandHandler h(store);
    REQUIRE(h.execute("") == "");
    REQUIRE(h.execute("   ") == "");
}