/**
 * Copyright (C) by J.Z. (04/03/2018 10:59)
 * Distributed under terms of the MIT license.
 */

#ifndef __DYNGRAPH_MGR_H__
#define __DYNGRAPH_MGR_H__

#include "stdafx.h"

class DynGraphMgr : public HyperANF {
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

    void reverseFanOut(const dir::DGraph& G, const int v,
                       std::unordered_set<int>& modified);

public:
    DynGraphMgr(const int p = 12) : HyperANF(p) {}

    void reset() { new_cc_edges_.clear(); }

    void addEdge(const int u, const int v);
    void addEdges(const std::vector<std::pair<int, int>>& edges);

    // Run DFS to identify CCs in DAG, return affected nodes
    std::vector<int> updateDAG();

}; /* DynGraphMgr */

#endif /* __DYNGRAPH_MGR_H__ */
