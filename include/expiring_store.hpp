#pragma once
#include "hash_table.hpp"
#include <cstdint>
#include <functional>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class ExpiringStore : public Store {
public:
    using NowFn = std::function<int64_t()>;  

    ExpiringStore();                        
    explicit ExpiringStore(NowFn now);   

    void set(const std::string& key, const std::string& value) override;
    std::optional<std::string> get(const std::string& key) const override;
    bool del(const std::string& key) override;

    bool expire(const std::string& key, int64_t seconds);
    int64_t ttl(const std::string& key) const; 
    bool persist(const std::string& key);
    size_t sweep();                          
    size_t size() const;

private:
    using HeapEntry = std::pair<int64_t, std::string>;  

    static int64_t default_now();
    void expire_if_needed(const std::string& key) const;

    NowFn now_;
    mutable HashTable data_;
    mutable std::unordered_map<std::string, int64_t> expiry_;
    mutable std::priority_queue<HeapEntry, std::vector<HeapEntry>, std::greater<HeapEntry>> heap_;
};