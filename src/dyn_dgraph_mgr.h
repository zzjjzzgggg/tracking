/**
 * Copyright (C) by J.Z. (04/03/2018 10:59)
 * Distributed under terms of the MIT license.
 */

#ifndef __DYNDGRAPH_MGR_H__
#define __DYNDGRAPH_MGR_H__

#include "stdafx.h"
#include "input_mgr.h"

class DynDGraphMgr : public InputMgr, public HyperANF {
private:
    dir::DGraph DAG_;
    std::unordered_set<int> cc_;  // CCs in current DAG
    std::stack<int> recycle_bin_;
    std::vector<std::pair<int, int>> new_cc_edges_;

private:
    // Get the CC node u belongs to; otherwise return u itself as the CC.
    int getCC(const int u);

    // Get an available pos from bits_.
    int getPos(const int cc);

    // mark pos in bits as an available possition
    void freePos(const int pos);

    void reverseFanOut(const dir::DGraph& G, const int cv,
                       std::unordered_set<int>& modified);

    template <class InputIter>
    double getGain(const int u, InputIter first, InputIter last) const;

public:
    DynDGraphMgr(const int p = 12) : HyperANF(p) {}

    void addEdge(const int u, const int v) override;
    void addEdges(const std::vector<std::pair<int, int>>& edges) override;

    // Run DFS to identify CCs in DAG, return affected nodes
    std::vector<int> updateDAG();

    std::vector<int> getAffectedNodes() override { return updateDAG(); }

    void clear(const bool deep_clear = false) override {}

    double getReward(const int node) const override { return estimate(node); }

    std::vector<int> getNodesAll() const override {
        std::vector<int> nodes;
        for (auto& pr : nd_cc_) nodes.push_back(pr.first);
        return nodes;
    }

    double getReward(const std::vector<int>& S) const override {
        return estimate(S.begin(), S.end());
    }

    double getReward(const std::unordered_set<int>& S) const override {
        return estimate(S.begin(), S.end());
    }

    double getGain(const int u, const std::vector<int>& S) const override {
        return getGain(u, S.begin(), S.end());
    }

    double getGain(const int u,
                   const std::unordered_set<int>& S) const override {
        return getGain(u, S.begin(), S.end());
    }

}; /* DynDGraphMgr */

template <class InputIter>
double DynDGraphMgr::getGain(const int u, InputIter first,
                             InputIter last) const {
    std::vector<uint64_t> tmp_bits(units_per_counter_, 0);
    for (; first != last; ++first)
        mergeCounter(tmp_bits.data(), cc_bitpos_.at(nd_cc_.at(*first)));
    double rwd_S = hll::count((uint8_t*)tmp_bits.data(), m_);
    mergeCounter(tmp_bits.data(), getCounterPos(u));
    double rwd_S_and_u = hll::count((uint8_t*)tmp_bits.data(), m_);
    return rwd_S_and_u - rwd_S;
}

#endif /* __DYNDGRAPH_MGR_H__ */
