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

    // Assign cc a position in bits_ vector and return the pos.
    int newPos(const int cc);

    inline bool exists(const int cc) const {
        return cc_bitpos_.find(cc) != cc_bitpos_.end();
    }

    // return true if c1 >= c2 in decreading topological order
    inline bool isGEq(const int c1, const int c2) const {
        return isGreaterEqual(cc_bitpos_.at(c1), cc_bitpos_.at(c2));
    }

    // merge counter of c2 to counter c1
    inline void mergeCC(const int c1, const int c2) {
        mergeCounter(cc_bitpos_.at(c1), cc_bitpos_.at(c2));
    }

    inline void genCounter(const int cc) { genHLLCounter(newPos(cc)); }

    // delete CC and mark its pos in bits_ as an available possition
    void delCC(const int cc);

    // do a BFS from node cv along its in-edges, each node in the BFS tree is
    // inserted into set modified.
    void reverseFanOut(const dir::DGraph& G, const int cv,
                       std::unordered_set<int>& modified);

    template <class InputIter>
    double getGain(const int u, const InputIter first,
                   const InputIter last) const;

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
    std::vector<int> getAffectedNodes() override{
    return std::vector<int>(affected_nodes_.begin(), affected_nodes_.end());
    }

    void clear(const bool deep = false) override;
    std::vector<int> getNodes() const override {
        std::vector<int> nodes;
        for (auto& pr : nd_cc_) nodes.push_back(pr.first);
        return nodes;
    }
    int getNumNodes() const override { return nd_cc_.size(); }
    double getReward(const int node) const override { return estimate(node); }
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

// implementations
template <class InputIter>
double DynDGraphMgr::getGain(const int u, const InputIter first,
                             const InputIter last) const {
    std::string key = "{}"_format(u);
    for (auto it = first; it != last; ++it) key.append("|{}"_format(*it));
    if (cache_.contains(key)) return cache_.get(key);

    ++oracle_calls_;
    double gain = 0;
    if (first == last) {
        gain = getReward(u);
    } else {
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
