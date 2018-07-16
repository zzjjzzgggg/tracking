/**
 * Copyright (C) by J.Z. (04/03/2018 14:43)
 * Distributed under terms of the MIT license.
 */
#include "dyn_dgraph_mgr_v2.h"

int DynDGraphMgr::getCC(const int u) {
    if (nd_cc_.find(u) != nd_cc_.end())
        return nd_cc_.at(u);
    else
        nd_cc_[u] = u;
    return u;
}

int DynDGraphMgr::newPos(const int cc) {
    int pos = bits_.size();
    bits_.resize(pos + units_per_counter_, 0);
    cc_bitpos_[cc] = pos;
    return pos;
}

void DynDGraphMgr::revBFS(const int cv) {
    affected_nodes_.insert(cv);
    std::queue<int> queue;
    std::unordered_set<int> visited{cv};
    queue.push(cv);
    while (!queue.empty()) {
        int cv = queue.front();
        queue.pop();
        const auto& nd = dag_[cv];
        for (auto ni = nd.beginInNbr(); ni != nd.endInNbr(); ++ni) {
            int cu = nd.getNbrID(ni);
            if (visited.find(cu) == visited.end()) {
                visited.insert(cu);
                if (!isGEq(cu, cv)) {
                    mergeCC(cu, cv);
                    affected_nodes_.insert(cu);
                    queue.push(cu);
                }
            }
        }
    }
}

void DynDGraphMgr::addEdge(const int u, const int v) {
    int cu = getCC(u), cv = getCC(v);
    // omit self-loop edges and edges already in DAG
    if (cu != cv && !dag_.isEdge(cu, cv)) {
        dag_.addEdge(cu, cv);
        if (!exists(cu) && !exists(cv)) {  // both u and v are new
            genCounter(cu);
            genCounter(cv);
            mergeCC(cu, cv);
            affected_nodes_.insert(cu);
            affected_nodes_.insert(cv);
        } else if (exists(cu)) {  // v is new
            genCounter(cv);
            affected_nodes_.insert(cv);
            mergeCC(cu, cv);
            revBFS(cu);
        } else if (exists(cv)) {  // u is new
            genCounter(cu);
            mergeCC(cu, cv);
            affected_nodes_.insert(cu);
        } else if (!isGEq(cu, cv)) {  // both exist and cu is not higher than cv
            mergeCC(cu, cv);
            revBFS(cu);
        }
    }
}

void DynDGraphMgr::clear(const bool deep) {
    InputMgr::clear(deep);
    if (deep) {
        HyperANF::clear();
        dag_.clear();
        cache_.clear();
    }
}
