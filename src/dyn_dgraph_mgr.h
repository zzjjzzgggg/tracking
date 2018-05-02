/**
 * Copyright (C) by J.Z. (04/03/2018 10:59)
 * Distributed under terms of the MIT license.
 */

#ifndef __DYNDGRAPH_MGR_H__
#define __DYNDGRAPH_MGR_H__

#include "input_mgr.h"

class DynDGraphMgr : public InputMgr, public HyperANF {
private:
    dir::DGraph dag_;
    std::stack<int> recycle_bin_;  // store available possitions in bits_
    std::vector<IntPr> new_cc_edges_;

    mutable lru::Cache<std::string, double> cache_{5, 0};

private:
    // Get the CC node u belongs to; otherwise return u itself as the CC.
    int getCC(const int u);

    // Get an available pos from bits_.
    int getPos(const int cc);

    // delete CC and mark its pos in bits_ as an available possition
    void deleteCC(const int cc);

    void reverseFanOut(const dir::DGraph& G, const int cv,
                       std::unordered_set<int>& modified);

    template <class InputIter>
    double getGain(const int u, const InputIter first, const InputIter last,
                   const bool check = true) const;

public:
    DynDGraphMgr(const int p = 10) : HyperANF(p), InputMgr() {}

    // copy constructor
    DynDGraphMgr(const DynDGraphMgr& o)
        : InputMgr(o), HyperANF(o), dag_(o.dag_), recycle_bin_(o.recycle_bin_),
          new_cc_edges_(o.new_cc_edges_) {}

    // copy assignment
    DynDGraphMgr& operator=(const DynDGraphMgr& o) {
        InputMgr::operator=(o);
        HyperANF::operator=(o);
        dag_ = o.dag_;
        recycle_bin_ = o.recycle_bin_;
        new_cc_edges_ = o.new_cc_edges_;
        return *this;
    }

    // addEdge will record new CC connections to new_cc_edges_.
    void addEdge(const int u, const int v) override;

    // Run DFS to identify CCs in DAG, return affected nodes.
    // will use new_cc_edges_.
    std::vector<int> getAffectedNodes() override;

    void clear(const bool deep = false) override;
    std::vector<int> getNodes() const override;
    int getNumNodes() const override { return nd_cc_.size(); }
    double getReward(const int node) const override;
    double getReward(const std::vector<int>& S) const override;
    double getReward(const std::unordered_set<int>& S) const override;
    double getGain(const int u, const std::vector<int>& S,
                   const bool check = true) const override;
    double getGain(const int u, const std::unordered_set<int>& S,
                   const bool check = true) const override;

}; /* DynDGraphMgr */

// implementations
template <class InputIter>
double DynDGraphMgr::getGain(const int u, const InputIter first,
                             const InputIter last, const bool check) const {
    std::string key = "{}"_format(u);
    for (auto it = first; it != last; ++it) key.append("|{}"_format(*it));
    if (cache_.contains(key)) return cache_.get(key);

    if (check) ++oracle_calls_;
    double gain = 0;
    if (first == last)
        gain = getReward(u);
    else {
        double rwd_S = 0;
        std::vector<uint64_t> tmp_bits(units_per_counter_, 0);
        for (auto it = first; it != last; ++it)
            mergeCounter(tmp_bits.data(), cc_bitpos_.at(nd_cc_.at(*it)));
        rwd_S = hll::count((uint8_t*)tmp_bits.data(), m_);
        mergeCounter(tmp_bits.data(), getCounterPos(u));

        gain = hll::count((uint8_t*)tmp_bits.data(), m_) - rwd_S;
    }

    cache_.insert(key, gain);
    return gain;
}

#endif /* __DYNDGRAPH_MGR_H__ */
