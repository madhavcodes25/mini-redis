#include "command_handler.hpp"
#include <sstream>
#include <optional>

CommandHandler::CommandHandler(HashTable& store) : store_(store) {}

std::string CommandHandler::execute(const std::string& command_line) {
    std::istringstream iss(command_line);
    std::string cmd;
    iss >> cmd; 

    if (cmd == "SET") {
        std::string key, value;
        if (iss >> key >> value) {
            store_.set(key, value);
            return "OK";
        }
        return "ERROR: SET requires a key and a value";
    } 
    else if (cmd == "GET") {
        std::string key;
        if (iss >> key) {
            auto val = store_.get(key); 
            // Check if the optional contains a value
            if (!val.has_value()) return "(nil)"; 
            return val.value(); // Extract the actual string
        }
        return "ERROR: GET requires a key";
    } 
    else if (cmd == "DEL") {
        std::string key;
        if (iss >> key) {
            store_.del(key);
            return "OK"; 
        }
        return "ERROR: DEL requires a key";
    } 
    else if (cmd == "EXISTS") {
        std::string key;
        if (iss >> key) {
            auto val = store_.get(key);
            // Return 1 if the optional has a value, 0 if it is empty
            return val.has_value() ? "(integer) 1" : "(integer) 0";
        }
        return "ERROR: EXISTS requires a key";
    }

    return "ERROR: Unknown command";
}