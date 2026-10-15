#pragma once
#include <list>
#include <optional>
#include <string>
#include <unordered_map>

class LruIndex {
public:
    void touch(const std::string& key);   
    void remove(const std::string& key);
    std::optional<std::string> evict();     
    size_t size() const;

private:
    std::list<std::string> order_;           
    std::unordered_map<std::string, std::list<std::string>::iterator> pos_;
};