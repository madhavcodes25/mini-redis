#pragma once
#include "expiring_store.hpp"
#include <string>

class CommandHandler {
public:
    explicit CommandHandler(ExpiringStore& store);
    std::string execute(const std::string& line);   // returns the reply text

private:
    ExpiringStore& store_;
};