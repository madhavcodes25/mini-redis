#include "command_handler.hpp"
#include <iostream>
#include <string>

int main() {
    ExpiringStore store(1000);                 // at most 1000 keys, then LRU eviction
    CommandHandler handler(store);
    std::cout << "mini-redis (type EXIT to quit)\n> ";
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "EXIT" || line == "QUIT") break;
        store.sweep();                         // drop keys that expired while idle
        std::string reply = handler.execute(line);
        if (!reply.empty()) std::cout << reply << "\n";
        std::cout << "> ";
    }
}