#include "lru_index.hpp"

void LruIndex::touch(const std::string& key) {
    auto it = pos_.find(key);
    if (it != pos_.end()) {
        order_.splice(order_.begin(), order_, it->second);  
    } else {
        order_.push_front(key);
        pos_[key] = order_.begin();
    }
}

void LruIndex::remove(const std::string& key) {
    auto it = pos_.find(key);
    if (it == pos_.end()) return;
    order_.erase(it->second);
    pos_.erase(it);
}

std::optional<std::string> LruIndex::evict() {
    if (order_.empty()) return std::nullopt;
    std::string key = order_.back();
    order_.pop_back();
    pos_.erase(key);
    return key;
}

size_t LruIndex::size() const { return order_.size(); }