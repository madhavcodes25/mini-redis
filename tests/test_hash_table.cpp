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

TEST_CASE("table grows when load factor passes 0.75") {
    HashTable table;
    REQUIRE(table.bucket_count() == 16);
    for (int i = 0; i < 12; i++) table.set("k" + std::to_string(i), "v");
    REQUIRE(table.bucket_count() == 16);
    table.set("k12", "v");
    REQUIRE(table.bucket_count() == 32); 
}

TEST_CASE("every key survives many resizes") {
    HashTable table;
    for (int i = 0; i < 10000; i++) table.set("key" + std::to_string(i), std::to_string(i));
    REQUIRE(table.size() == 10000);
    for (int i = 0; i < 10000; i++) {
        REQUIRE(table.get("key" + std::to_string(i)) == std::to_string(i));
    }
}

TEST_CASE("delete still works after a resize") {
    HashTable table;
    for (int i = 0; i < 100; i++) table.set("key" + std::to_string(i), "v");
    REQUIRE(table.del("key50") == true);
    REQUIRE(table.get("key50") == std::nullopt);
    REQUIRE(table.size() == 99);
}

TEST_CASE("size counts keys, not writes") {
    HashTable table;
    table.set("a", "1");
    table.set("a", "2");
    REQUIRE(table.size() == 1);
    table.del("a");
    REQUIRE(table.size() == 0);
}