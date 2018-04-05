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

    // venues that their gains possibly changed
    std::unordered_set<int> affected_venues_;

private:
    template <class InputIter>
    double getGain(const int v, InputIter first, InputIter last) const;

    template <class InputIter>
    double getReward(InputIter first, InputIter last) const;

public:
    DynBGraphMgr() {}
    virtual ~DynBGraphMgr() {}

    void addEdges(const std::vector<std::pair<int, int>>& edges) override {
        affected_venues_.clear();
        for (auto& pr : edges) {
            int u = pr.first, v = pr.second;  // u: user, v: venue
            if (!graph_.isEdge(u, v)) {
                graph_.addEdge(u, v);
                affected_venues_.insert(v);
            }
        }
    }

    std::vector<int> getAffectedNodes() override {
        std::vector<int> nodes(affected_venues_.begin(),
                               affected_venues_.end());
        return nodes;
    }

    double getReward(const int node) const override {
        return graph_.getNodeR(node).getDeg();
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
double DynBGraphMgr::getGain(const int node, InputIter first,
                             InputIter last) const {
    std::unordered_set<int> covered;

    // covered users by S
    for (; first != last; ++first) {
        if (graph_.isNodeL(*first)) {
            const auto& nd = graph_.getNodeR(*first);
            covered.insert(nd.beginNbr(), nd.endNbr());
        }
    }
    int rwd_S = covered.size();

    // covered users by node
    if (graph_.isNodeL(node)) {
        const auto& nd = graph_.getNodeR(node);
        covered.insert(nd.beginNbr(), nd.endNbr());
    }
    int rwd_S_and_u = covered.size();

    return rwd_S_and_u - rwd_S;
}

template <class InputIter>
double DynBGraphMgr::getReward(InputIter first, InputIter last) const {
    std::unordered_set<int> covered;
    for (; first != last; ++first) {
        int v = *first;
        if (graph_.isNodeL(v)) {
            const auto& nd_v = graph_.getNodeR(v);
            covered.insert(nd_v.beginNbr(), nd_v.endNbr());
        }
    }
    return covered.size();
}

#endif /* __DYN_BGRAPH_MGR_H__ */
