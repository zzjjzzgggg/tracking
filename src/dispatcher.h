#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__

#include "stdafx.h"
#include "config.h"
#include "walk_storage.h"
#include "walk_analyzer.h"

class Dispatcher {
private:
    Config* conf_;
    WalkStorage* storage_;
    WalkAnalyzer* analyzer_;

    vector<vector<long> *> buf_odd_, buf_eve_;
    std::mutex* mutex_vec_;
    std::mutex mutex_;

private:
    /**
     * If the walker hits absorbing node, or reaches the maximum walk length,
     * then terminates.
     */
    inline bool stopCond(const int node, const long& walk) {
        return node == conf_->absb_node || Encoder::getHop(walk) >= conf_->hops;
    }

public:
    Dispatcher(Config* conf, WalkAnalyzer* analyzer = nullptr,
               WalkStorage* storage = nullptr)
        : conf_(conf), analyzer_(analyzer), storage_(storage) {
        buf_odd_.resize(conf_->n_bkts, nullptr);
        buf_eve_.resize(conf_->n_bkts, nullptr);
        mutex_vec_ = new std::mutex[conf_->n_bkts];
    }

    virtual ~Dispatcher() { delete[] mutex_vec_; }

    void bind(const int bkt_offset, vector<vector<long>>& buf_odd,
              vector<vector<long>>& buf_eve) {
        for (size_t i = 0; i < buf_odd.size(); i++) {
            if (bkt_offset + i < conf_->n_bkts)
                buf_odd_[bkt_offset + i] = &buf_odd[i];
        }
        for (size_t i = 0; i < buf_eve.size(); i++) {
            if (bkt_offset + i < conf_->n_bkts)
                buf_eve_[bkt_offset + i] = &buf_eve[i];
        }
    }

    void dispatch(const long& walk, const int to_node, const bool write_even) {
        long new_walk = Encoder::upcode(walk, to_node);
        int bucket = GET_BUCKET(to_node);

        std::unique_lock<std::mutex> lock_all(mutex_);
        if (storage_ != nullptr) storage_->store(bucket, new_walk);
        if (analyzer_ != nullptr) analyzer_->analyze(to_node, new_walk);
        lock_all.unlock();

        std::unique_lock<std::mutex> lock_bkt(mutex_vec_[bucket]);
        if (!stopCond(to_node, new_walk)) {
            if (write_even)
                buf_eve_[bucket]->push_back(new_walk);
            else
                buf_odd_[bucket]->push_back(new_walk);
        }
    }
};

#endif /* __DISPATCHER_H__ */
