#pragma once
#include "store.hpp"
#include <list>
#include <vector>
#include <utility>

class HashTable : public Store {
public:
    HashTable();
    void set(const std::string& key, const std::string& value) override;
    std::optional<std::string> get(const std::string& key) const override;
    bool del(const std::string& key) override;
    size_t size() const;
    size_t bucket_count() const;

private:
    static constexpr double kMaxLoadFactor = 0.75;
    void resize(size_t new_bucket_count);
    std::vector<std::list<std::pair<std::string, std::string>>> buckets_;
    size_t size_;
    size_t get_index(const std::string& key) const;
};