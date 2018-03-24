#ifndef __HITTING_TIME_ANALYZER_H__
#define __HITTING_TIME_ANALYZER_H__

#include "stdafx.h"
#include "walk_analyzer.h"

/**
 * Record the hitting time of the walks that hit target n
 */
class HittingTimeAnalyzer : public WalkAnalyzer {
public:
    unordered_map<long, int> walk_ht_;

public:
    HittingTimeAnalyzer(Config* conf) : WalkAnalyzer(conf) {}

    /**
     * node: the node that the walk currently resides;
     * walk: new walk
     */
    void analyze(const int node, const long walk) override {
        if (node == conf_->absb_node)
            walk_ht_[Encoder::getWalkId(walk)] = Encoder::getHop(walk);
    }

    void clear() { walk_ht_.clear(); }

    /**
     * sfx_fnm: {}_{}.gz
     */
    void saveResult(const int trial) const {
        string nd_ht_fnm = osutils::join(
            conf_->dir, "rw_stats/{}_{}_bin.gz"_format(conf_->walks, trial));
        auto out_ptr = ioutils::getIOOut(nd_ht_fnm);
        out_ptr->save((int)walk_ht_.size() * 2);
        for (auto& pr : walk_ht_) {
            out_ptr->save(Encoder::getSource(pr.first));
            out_ptr->save(pr.second);
        }
        printf("node hitting time saved to %s\n", nd_ht_fnm.c_str());
    }
};

/**
 * Record the steps of all walks, no matter whether or not the walk hits n.
 */
class WalkStepAnalyzer : public HittingTimeAnalyzer {
public:
    WalkStepAnalyzer(Config* conf) : HittingTimeAnalyzer(conf) {}

    /**
     * If the node hits the target node, the highest bit in "hop" is set 1.
     *
     * node: the node where the walker currently resides.
     * walk: new walk.
     */
    void analyze(const int node, const long walk) override {
        int hop = Encoder::getHop(walk);
        if (node == conf_->absb_node) hop |= 0x80;
        walk_ht_[Encoder::getWalkId(walk)] = hop;
    }
};

#endif /* __HITTING_TIME_ANALYZER_H__ */
