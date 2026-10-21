#include <iostream>
#include <string>
#include "command_handler.hpp"
#include "hash_table.hpp"

int main() {
    HashTable store;
    CommandHandler handler(store);
    std::string line;

    std::cout << "Mini-Redis CLI. Type EXIT to quit.\n";

    while (true) {
        std::cout << "127.0.0.1:6379> ";
        
        // Safely grab the whole line
        if (!std::getline(std::cin, line)) break; 
        
        if (line == "EXIT") {
            break;
        }
        
        if (line.empty()) {
            continue;
        }

        // Pass the raw line to the parser and print the result
        std::cout << handler.execute(line) << std::endl;
    }

    return 0;
}