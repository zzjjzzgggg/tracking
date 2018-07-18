/**
 * Copyright (C) by J.Z. (2018-07-17 14:41)
 * Distributed under terms of the MIT license.
 */

#ifndef __DIM_STREAM_H__
#define __DIM_STREAM_H__

#include "../stdafx.h"
#include "dim.hpp"

struct hashpr {
    size_t operator()(const IntPr& edge) const {
        return (std::hash<int>()(edge.first) << 32) |
               std::hash<int>()(edge.second);
    }
};


template <class InputMgr>
class DIMStream {
private:
    int L_, cur_ = 0;
    std::vector<IntPrV> edge_buf_;

    // edge -> number of interactions
    std::unordered_map<IntPr, int, hashpr> edge_num_;
    // node -> (in-deg, out-deg)
    std::unordered_map<int, IntPr> nd_deg_;

    DIM dim_;

private:
    /**
     * Return true if node u already exists.
     */
    bool exists(const int u) const { return nd_deg_.find(u) != nd_deg_.end(); }

    /**
     * Return true if edge (u, v) exists.
     */
    bool exists(const int u, const int v) const {
        return edge_num_.find(IntPr(u, v)) != edge_num_.end();
    }

    double prob(const int x) const { return 2 / (1 + std::exp(-.2 * x)) - 1; }

public:
    DIMStream(const int L, const int beta) : L_(L) {
        dim_.init();
        dim_.set_beta(beta);
        edge_buf_.resize(L);
    }

    void addEdge(const int u, const int v, const int l) {
        edge_buf_[(cur_ + l - 1) % L_].emplace_back(u, v);
        if (!exists(u) && !exists(v)) {  // both u and v are new
            dim_.insert(u);
            dim_.insert(v);
            dim_.insert(u, v, prob(1));
            nd_deg_[u] = IntPr(0, 1);
            nd_deg_[v] = IntPr(1, 0);
            edge_num_[IntPr(u, v)] = 1;
        } else if (!exists(u)) {  // u is new
            dim_.insert(u);
            dim_.insert(u, v, prob(1));
            nd_deg_[u] = IntPr(0, 1);
            ++nd_deg_[v].first;
            edge_num_[IntPr(u, v)] = 1;
        } else if (!exists(v)) {  // v is new
            dim_.insert(v);
            dim_.insert(u, v, prob(1));
            ++nd_deg_[u].second;
            nd_deg_[v] = IntPr(1, 0);
            edge_num_[IntPr(u, v)] = 1;
        } else {  // both u and v exist
            int x = ++edge_num_[IntPr(u, v)];
            ++nd_deg_[u].second;  // out-deg of u
            ++nd_deg_[v].first;   // in-deg of v
            if (x == 1)
                dim_.insert(u, v, prob(1));
            else
                dim_.change(u, v, prob(x));
        }
    }

    std::vector<int> infmax(const int budget) {
        int k = std::min(budget, (int)(nd_deg_.size()));
        return dim_.infmax(k);
    }

    double infest(std::vector<int>& nodes) { return dim_.infest(nodes); }

    InputMgr getInputMgr() const {
        InputMgr input_mgr;
        for (auto& edges : edge_buf_) input_mgr.addEdges(edges);
        input_mgr.getAffectedNodes();
        return input_mgr;
    }

    void next() {
        for (auto& edge : edge_buf_[cur_]) {
            int x = --edge_num_[edge], u = edge.first, v = edge.second;
            if (x == 0) {
                dim_.erase(u, v);
                edge_num_.erase(edge);
            } else {
                dim_.change(u, v, prob(x));
            }
            --nd_deg_[u].second;
            if (nd_deg_[u] == IntPr(0, 0)) {
                nd_deg_.erase(u);
                dim_.erase(u);
            }
            --nd_deg_[v].first;
            if (nd_deg_[v] == IntPr(0, 0)) {
                nd_deg_.erase(v);
                dim_.erase(v);
            }
        }
        edge_buf_[cur_].clear();
        cur_ = (cur_ + 1) % L_;
    }

}; /* DIMStream */

#endif /* __DIM_STREAM_H__ */
