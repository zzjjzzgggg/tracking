#ifndef __WALK_ENGINE_H__
#define __WALK_ENGINE_H__

#include "stdafx.h"
#include "graph_driver.h"
#include "walk_storage.h"
#include "walk_analyzer.h"
#include "dispatcher.h"

class WalkEngine {
private:
    Config* conf_;
    GraphDriver* driver_;
    Dispatcher* dispatcher_;

    // manage the walks in the given bucket
    int bkt_offset_;

    // buf_odd_/buf_eve_: walks in odd/even hops
    vector<vector<long>> buf_odd_, buf_eve_;

public:
    WalkEngine(Config* config, GraphDriver* driver)
        : conf_(config), driver_(driver) {
        buf_odd_.resize(conf_->n_bkts_a_cpu);
        buf_eve_.resize(conf_->n_bkts_a_cpu);
    }

    WalkEngine(const WalkEngine&) = delete;
    WalkEngine& operator=(const WalkEngine&) = delete;

    WalkEngine(WalkEngine&& other)
        : conf_(std::move(other.conf_)), driver_(std::move(other.driver_)),
          dispatcher_(std::move(other.dispatcher_)),
          bkt_offset_(std::move(other.bkt_offset_)),
          buf_odd_(std::move(other.buf_odd_)),
          buf_eve_(std::move(other.buf_eve_)) {}

    WalkEngine& operator=(WalkEngine&& other) {
        conf_ = std::move(other.conf_);
        driver_ = std::move(other.driver_);
        dispatcher_ = std::move(other.dispatcher_);
        bkt_offset_ = std::move(other.bkt_offset_);
        buf_odd_ = std::move(other.buf_odd_);
        buf_eve_ = std::move(other.buf_eve_);
        return *this;
    }

    void registerDispatcher(const int bkt_offset, Dispatcher* dispatcher);

    /**
     * For each node in the bucket, initialize a collection of walks, stored in
     * "buf_odd_".
     */
    void initWalks();

    /**
     * Set walks starting from the given node.
     * node: the node where the walker currently resides
     * walks_to_resume: walks to resume from the given node, only receive rvalue
     * to improve efficiency.
     */
    void setWalksFrom(const int node, vector<long>&& walks_to_resume);

    /**
     * Process each walk in each bucket.
     */
    void startWalking();

    /**
     * Walking asynchronously.
     *
     * Return true if bucket buffers are all empty.
     */
    bool walkSync(const bool read_odd);
};

#endif
