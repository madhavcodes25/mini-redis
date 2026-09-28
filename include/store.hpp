#pragma once
#include <string>
#include <optional>

class Store {
public:
    virtual void set(const std::string& key, const std::string& value) = 0;
    virtual std::optional<std::string> get(const std::string& key) const = 0;
    virtual bool del(const std::string& key) = 0;
    virtual ~Store() = default;
};