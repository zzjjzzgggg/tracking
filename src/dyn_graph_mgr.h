/**
 * Copyright (C) by J.Z. (04/25/2018 08:43)
 * Distributed under terms of the MIT license.
 */

#ifndef __DYN_GRAPH_MGR_H__
#define __DYN_GRAPH_MGR_H__

#include "input_mgr.h"

class DynGraphMgr : public InputMgr {
private:
    bi::BGraph graph_;  // nodes: users, edge (u,v): u can influence v

private:
    template <class InputIter>
    double getReward(InputIter first, InputIter last) const;

    template <class InputIter>
    double getGain(const int v, InputIter first, InputIter last) const;

public:
    DynGraphMgr() : InputMgr() {}

    // Copy constructor
    DynGraphMgr(const DynGraphMgr& o) : InputMgr(o), graph_(o.graph_) {}

    // copy assignment
    DynGraphMgr& operator=(const DynGraphMgr& o) {
        InputMgr::operator=(o);
        graph_ = o.graph_;
        return *this;
    }

    void addEdge(const int u, const int v) override;
    void clear(const bool deep = false) override;

    int getNumNodes() const override { return graph_.getNodesL(); }

    std::vector<int> getNodes() const override;

    double getReward(const int node) const override {
        if (!graph_.isNodeL(node)) return 1;
        oracle_calls_++;
        return graph_.getNodeL(node).getDeg() + 1;
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

}; /* DynGraphMgr */

// implementations
template <class InputIter>
double DynGraphMgr::getReward(InputIter first, InputIter last) const {
    if (first == last) return 0;
    oracle_calls_++;
    std::unordered_set<int> covered;
    for (; first != last; ++first) {
        int u = *first;
        covered.insert(u);
        if (graph_.isNodeL(u)) {
            const auto& nd_u = graph_.getNodeL(u);
            covered.insert(nd_u.beginNbr(), nd_u.endNbr());
        }
    }
    return covered.size();
}

template <class InputIter>
double DynGraphMgr::getGain(const int node, InputIter first,
                            InputIter last) const {
    if (first == last) return getReward(node);
    oracle_calls_++;
    std::unordered_set<int> covered;

    // covered users by S
    for (; first != last; ++first) {
        covered.insert(*first);
        if (graph_.isNodeL(*first)) {
            const auto& nd = graph_.getNodeL(*first);
            covered.insert(nd.beginNbr(), nd.endNbr());
        }
    }
    int rwd_S = covered.size();

    // covered users by node
    covered.insert(node);
    if (graph_.isNodeL(node)) {
        const auto& nd = graph_.getNodeL(node);
        covered.insert(nd.beginNbr(), nd.endNbr());
    }
    int rwd_S_and_u = covered.size();

    return rwd_S_and_u - rwd_S;
}

#endif /* __DYN_GRAPH_MGR_H__ */
