#include <iostream>
#include <cassert>
#include "command_handler.hpp"
#include "hash_table.hpp"

int main() {
    HashTable store;
    CommandHandler handler(store);

    // 1. Test SET
    assert(handler.execute("SET name madhav") == "OK");

    // 2. Test GET
    assert(handler.execute("GET name") == "madhav");

    // 3. Test EXISTS
    assert(handler.execute("EXISTS name") == "(integer) 1");
    assert(handler.execute("EXISTS unknown") == "(integer) 0");

    // 4. Test DEL
    assert(handler.execute("DEL name") == "OK");
    assert(handler.execute("EXISTS name") == "(integer) 0");
    assert(handler.execute("GET name") == "(nil)");

    std::cout << "All CommandHandler tests passed successfully!" << std::endl;
    return 0;
}