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

    // Copy constructor
    DynBGraphMgr(const DynBGraphMgr& o)
        : graph_(o.graph_), affected_venues_(o.affected_venues_) {}

    // copy assignment
    DynBGraphMgr& operator=(const DynBGraphMgr& o) {
        graph_ = o.graph_;
        affected_venues_ = o.affected_venues_;
        return *this;
    }

    void addEdge(const int u, const int v) override {
        if (!graph_.isEdge(u, v)) {
            graph_.addEdge(u, v);
            affected_venues_.insert(v);
        }
    }

    void addEdges(const std::vector<std::pair<int, int>>& edges) override {
        for (auto& pr : edges) {
            int u = pr.first, v = pr.second;
            addEdge(u, v);
        }
    }

    void clear(const bool deep = false) override {
        affected_venues_.clear();
        if (deep) graph_.clear();
    }

    std::vector<int> getAffectedNodes() override {
        std::vector<int> nodes(affected_venues_.begin(),
                               affected_venues_.end());
        return nodes;
    }

    double getReward(const int node) const override {
        return graph_.getNodeR(node).getDeg();
    }

    std::vector<int> getNodesAll() const override {
        std::vector<int> nodes;
        for (auto it = graph_.beginNIR(); it != graph_.endNIR(); ++it)
            nodes.push_back(it->first);
        return nodes;
    }

    double getReward(const std::vector<int>& S) const override {
        if (S.empty()) return 0;
        return getReward(S.begin(), S.end());
    }

    double getReward(const std::unordered_set<int>& S) const override {
        if (S.empty()) return 0;
        return getReward(S.begin(), S.end());
    }

    double getGain(const int u, const std::vector<int>& S) const override {
        if (S.empty()) return getReward(u);
        return getGain(u, S.begin(), S.end());
    }
    double getGain(const int u,
                   const std::unordered_set<int>& S) const override {
        if (S.empty()) return getReward(u);
        return getGain(u, S.begin(), S.end());
    }

    void getGraphStat() const {
        printf("nodes L: %d, nodes R: %d, edges: %d\n", graph_.getNodesL(),
               graph_.getNodesR(), graph_.getEdges());
    }

}; /* DynBGraphMgr */

template <class InputIter>
double DynBGraphMgr::getGain(const int node, InputIter first,
                             InputIter last) const {
    std::unordered_set<int> covered;

    // covered users by S
    for (; first != last; ++first) {
        if (graph_.isNodeR(*first)) {
            const auto& nd = graph_.getNodeR(*first);
            covered.insert(nd.beginNbr(), nd.endNbr());
        }
    }
    int rwd_S = covered.size();

    // covered users by node
    if (graph_.isNodeR(node)) {
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
        if (graph_.isNodeR(v)) {
            const auto& nd_v = graph_.getNodeR(v);
            covered.insert(nd_v.beginNbr(), nd_v.endNbr());
        }
    }
    return covered.size();
}

#endif /* __DYN_BGRAPH_MGR_H__ */
