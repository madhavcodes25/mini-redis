#pragma once
#include <string>
#include "hash_table.hpp"

class CommandHandler {
public:
    // Initialize with a reference to the existing storage mechanism
    CommandHandler(HashTable& store);

    // Parses a single command line string and returns the output
    std::string execute(const std::string& command_line);

private:
    HashTable& store_;
};