#include <catch2/catch_test_macros.hpp>
#include "hash_table.hpp"

TEST_CASE("set and get a key") {
    HashTable table;
    table.set("a", "1");
    REQUIRE(table.get("a") == "1");
}

TEST_CASE("get missing key returns nullopt") {
    HashTable table;
    REQUIRE(table.get("nope") == std::nullopt);
}

TEST_CASE("del returns true for an existing key and false for a missing one") {
    HashTable table;
    table.set("a", "1");
    REQUIRE(table.del("a") == true);
    REQUIRE(table.del("a") == false);
    REQUIRE(table.del("never-existed") == false);
}

TEST_CASE("setting an existing key overwrites its value") {
    HashTable table;
    table.set("a", "1");
    table.set("a", "2");
    REQUIRE(table.get("a") == "2");
}

TEST_CASE("an empty string is a valid value, not a missing key") {
    HashTable table;
    table.set("a", "");
    REQUIRE(table.get("a") == "");
    REQUIRE(table.get("a").has_value());
}

TEST_CASE("many keys all survive, including collisions") {
    HashTable table;
    for (int i = 0; i < 1000; i++) {
        table.set("key" + std::to_string(i), std::to_string(i));
    }
    for (int i = 0; i < 1000; i++) {
        REQUIRE(table.get("key" + std::to_string(i)) == std::to_string(i));
    }
}