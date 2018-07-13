/**
 * Copyright (C) by J.Z. (04/05/2018 17:16)
 * Distributed under terms of the MIT license.
 */

#ifndef __DYN_BGRAPH_MGR_H__
#define __DYN_BGRAPH_MGR_H__

#include "input_mgr.h"

/**
 * Handling dynamic bipartite graph kind of data, such as user check-ins and
 * author conference.
 */
class DynBGraphMgr : public InputMgr {
private:
    bi::BGraph graph_;  // L: user set, R: location set, location popularity

    mutable lru::Cache<std::string, double> cache_{5, 0};

private:
    template <class InputIter>
    double getGain(const int v, const InputIter first,
                   const InputIter last) const;

    template <class InputIter>
    double getReward(const InputIter first, const InputIter last) const;

public:
    DynBGraphMgr() : InputMgr() {}

    // Copy constructor
    DynBGraphMgr(const DynBGraphMgr& o) : InputMgr(o), graph_(o.graph_) {}

    // copy assignment
    DynBGraphMgr& operator=(const DynBGraphMgr& o) {
        InputMgr::operator=(o);
        graph_ = o.graph_;
        return *this;
    }

    void addEdge(const int u, const int v) override {
        graph_.addEdge(u, v);
        affected_nodes_.insert(v);
    }

    void clear(const bool deep = false) override {
        InputMgr::clear(deep);
        if (deep) graph_.clear();
    }

    int getNumNodes() const override { return graph_.getNodesR(); }

    std::vector<int> getNodes() const override {
        std::vector<int> nodes;
        nodes.reserve(graph_.getNodesR());
        for (auto it = graph_.beginNIR(); it != graph_.endNIR(); ++it)
            nodes.push_back(it->first);
        return nodes;
    }

    double getReward(const int v) const override {
        return graph_.getNodeR(v).getDeg();
    }

    double getReward(const std::vector<int>& S) const override {
        return getReward(S.begin(), S.end());
    }

    double getReward(const std::unordered_set<int>& S) const override {
        return getReward(S.begin(), S.end());
    }

    double getGain(const int u, const std::vector<int>& S) const override {
        return getGain(u, S.begin(), S.end());
    }
    double getGain(const int u,
                   const std::unordered_set<int>& S) const override {
        return getGain(u, S.begin(), S.end());
    }

}; /* DynBGraphMgr */

template <class InputIter>
double DynBGraphMgr::getGain(const int v, const InputIter first,
                             const InputIter last) const {
    std::string key = "{}"_format(v);
    for (auto it = first; it != last; ++it) key.append("|{}"_format(*it));
    if (cache_.contains(key)) return cache_.get(key);

    oracle_calls_++;

    double gain = 0;
    if (first == last)
        gain = getReward(v);
    else {
        std::unordered_set<int> covered;

        // covered users by S
        for (auto it = first; it != last; ++it) {
            if (graph_.isNodeR(*it)) {
                const auto& nd = graph_.getNodeR(*it);
                covered.insert(nd.beginNbr(), nd.endNbr());
            }
        }
        int rwd_S = covered.size();

        // covered users by v
        if (graph_.isNodeR(v)) {
            const auto& nd = graph_.getNodeR(v);
            covered.insert(nd.beginNbr(), nd.endNbr());
        }
        gain = covered.size() - rwd_S;
    }

    cache_.insert(key, gain);
    return gain;
}

template <class InputIter>
double DynBGraphMgr::getReward(const InputIter first,
                               const InputIter last) const {
    if (first == last) return 0;
    std::unordered_set<int> covered;
    for (auto it = first; it != last; ++it) {
        int v = *it;
        if (graph_.isNodeR(v)) {
            const auto& nd_v = graph_.getNodeR(v);
            covered.insert(nd_v.beginNbr(), nd_v.endNbr());
        }
    }
    return covered.size();
}

#endif /* __DYN_BGRAPH_MGR_H__ */
