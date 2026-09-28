#include "hash_table.hpp"
#include <functional>

HashTable::HashTable() : buckets_(16), size_(0) {}

size_t HashTable::get_index(const std::string& key) const {
    return std::hash<std::string>{}(key) % buckets_.size();
}

void HashTable::set(const std::string& key, const std::string& value) {
    size_t index = get_index(key);
    for (auto& entry : buckets_[index]) {
        if (entry.first == key) {
            entry.second = value;
            return;
        }
    }
    buckets_[index].push_back({key, value});
    size_++;
}

std::optional<std::string> HashTable::get(const std::string& key) const {
    size_t index = get_index(key);
    for (const auto& entry : buckets_[index]) {
        if (entry.first == key) {
            return entry.second;
        }
    }
    return std::nullopt;
}

bool HashTable::del(const std::string& key) {
    size_t index = get_index(key);
    auto& bucket = buckets_[index];
    for (auto it = bucket.begin(); it != bucket.end(); ++it) {
        if (it->first == key) {
            bucket.erase(it);
            size_--;
            return true;
        }
    }
    return false;
}

size_t HashTable::size() const {
    return size_;
}