#ifndef LRUCACHE_H_
#define LRUCACHE_H_

#include <memory>
#include <map>
#include <list>

template <typename Key, typename Value>
class LRUCache {
private:
    typedef std::list<Key> HistoryType;
    typedef typename HistoryType::iterator HistoryTypeIterator;
    typedef std::map<Key, std::pair<Value, HistoryTypeIterator>> CacheType;
    typedef typename CacheType::iterator CacheTypeIterator;

    HistoryType history_;
    CacheType cache_;
    size_t capacity_;

private:
    void evict(size_t num_elements = 1) {
        if (num_elements > history_.size()) {
            num_elements = history_.size();
        }
        while (num_elements--) {
            auto it = cache_.find(history_.front());
            cache_.erase(it);
            history_.pop_front();
        }
    }

    void updateHistory(const CacheTypeIterator& it) {
        history_.splice(history_.end(), history_, it->second.second);
    }

public:
    // if capacity = 0 -> unlimited capacity
    LRUCache(size_t capacity) : capacity_(capacity) {}

    ~LRUCache() {}

    LRUCache(const LRUCache& other) : capacity_(other.capacity_) {
        cache_ = other.cache_;
        history_ = other.history_;
        // Adjust Iterators
        for (auto it = history_.begin(); it != history_.end(); ++it)
            cache_.find(*it)->second.second = it;
    }

    LRUCache(LRUCache&& other) : capacity_(other.capacity_) {
        history_ = std::move(other.history_);
        cache_ = std::move(other.cache_);
    }

    void dropCache(size_t num_remaining = 0) {
        size_t elements_dropped;
        if (num_remaining == 0) {
            elements_dropped = history_.size();
            cache_.clear();
            history_.clear();
        } else if (history_.size() > num_remaining) {
            elements_dropped = history_.size() - num_remaining;
            evict(elements_dropped);
        }
    }

    void clear() { dropCache(); }
    size_t size() const { return history_.size(); }
    size_t capacity() const { return capacity_; }

    void setMaxCapacity(const size_t& capacity) {
        // problematic case: shrink cache_
        if (capacity < capacity_ && capacity != 0) dropCache(capacity);
        capacity_ = capacity;
    }

    /**
     * Inserts new element. If the element already exists, it will be *
     * overwritten. Returns a reference to the inserted object.
     */
    Value& insert(const Key& id, Value&& c) {
        // Check if the element is already existing
        auto it = cache_.find(id);
        if (it != cache_.end()) {
            // If the element exists, overwrite it
            // it->second.first = std::move(c);
            updateHistory(it);
        } else {
            if (capacity_ > 0 && history_.size() == capacity_) evict();
            auto end = history_.insert(history_.end(), id);
            auto newelem = cache_.insert(
                std::make_pair(id, std::make_pair(std::move(c), end)));
            it = newelem.first;
        }
        return it->second.first;
    }

    Value& operator[](const Key& id) {
        auto it = cache_.find(id);
        if (it != cache_.end()) {
            updateHistory(it);
            return it->second.first;
        }
        // Create new empty element. Move is called implicitly
        return insert(id, Value());
    }

    bool contains(const Key& id) const {
        return cache_.find(id) != cache_.end();
    }
};
#endif
