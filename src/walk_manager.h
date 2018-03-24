#ifndef __WALK_MANAGER_H__
#define __WALK_MANAGER_H__

#include "stdafx.h"
#include "walk_engine.h"

/**
 * WalkManager manages several WalkEngines
 */
class WalkManager {
private:
    Config* conf_;
    Dispatcher* dispatcher_;

    vector<WalkEngine> walk_engines_;

public:
    WalkManager(Config* conf, Dispatcher* dispatcher)
        : conf_(conf), dispatcher_(dispatcher) {
        walk_engines_.reserve(conf_->n_cpus);
    }

    void addWalkEngine(GraphDriver* driver) {
        int id = walk_engines_.size();
        walk_engines_.emplace_back(conf_, driver);

        int offset = id * conf_->n_bkts_a_cpu;
        walk_engines_[id].registerDispatcher(offset, dispatcher_);
    }

    void initWalks() {
        for (auto& mgr : walk_engines_) mgr.initWalks();
    }

    void setWalksFrom(const int node, vector<long>&& walks_to_resume) {
        int cpu = node / conf_->n_nodes_a_cpu;
        walk_engines_[cpu].setWalksFrom(node, std::move(walks_to_resume));
    }

    void startWalking() {
        bool finished = false, walk_odd = true;
        vector<std::future<bool>> futures;
        while (!finished) {
            for (auto& mgr : walk_engines_)
                futures.push_back(std::async(
                    std::launch::async, &WalkEngine::walkSync, &mgr, walk_odd));
            finished = true;
            for (auto& f : futures) finished &= f.get();
            futures.clear();
            walk_odd = !walk_odd;
        }
    }
};

#endif /* __WALK_MANAGER_H__ */
