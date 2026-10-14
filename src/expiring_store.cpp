#include "expiring_store.hpp"
#include <chrono>

int64_t ExpiringStore::default_now() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

ExpiringStore::ExpiringStore() : now_(default_now) {}
ExpiringStore::ExpiringStore(NowFn now) : now_(std::move(now)) {}

void ExpiringStore::expire_if_needed(const std::string& key) const {
    auto it = expiry_.find(key);
    if (it != expiry_.end() && it->second <= now_()) {
        data_.del(key);
        expiry_.erase(it);
    }
}

void ExpiringStore::set(const std::string& key, const std::string& value) {
    data_.set(key, value);
    expiry_.erase(key);            
}

std::optional<std::string> ExpiringStore::get(const std::string& key) const {
    expire_if_needed(key);
    return data_.get(key);
}

bool ExpiringStore::del(const std::string& key) {
    expire_if_needed(key);
    expiry_.erase(key);
    return data_.del(key);
}

bool ExpiringStore::expire(const std::string& key, int64_t seconds) {
    expire_if_needed(key);
    if (!data_.get(key)) return false;
    int64_t when = now_() + seconds * 1000;
    expiry_[key] = when;
    heap_.emplace(when, key);
    return true;
}

int64_t ExpiringStore::ttl(const std::string& key) const {
    expire_if_needed(key);
    if (!data_.get(key)) return -2;
    auto it = expiry_.find(key);
    if (it == expiry_.end()) return -1;
    int64_t remaining_ms = it->second - now_();
    return (remaining_ms + 999) / 1000;    
}

bool ExpiringStore::persist(const std::string& key) {
    expire_if_needed(key);
    return expiry_.erase(key) > 0;
}

size_t ExpiringStore::sweep() {
    size_t removed = 0;
    int64_t now = now_();
    while (!heap_.empty() && heap_.top().first <= now) {
        HeapEntry top = heap_.top();
        heap_.pop();
        auto it = expiry_.find(top.second);
        if (it != expiry_.end() && it->second == top.first) {  
            data_.del(top.second);
            expiry_.erase(it);
            removed++;
        }
    }
    return removed;
}

size_t ExpiringStore::size() const { return data_.size(); }