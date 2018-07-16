/**
 * Copyright (C) by J.Z. (04/03/2018 14:43)
 * Distributed under terms of the MIT license.
 */
#include "dyn_dgraph_mgr.h"

int DynDGraphMgr::getCC(const int u) {
    if (nd_cc_.find(u) != nd_cc_.end())
        return nd_cc_.at(u);
    else
        nd_cc_[u] = u;
    return u;
}

int DynDGraphMgr::newPos(const int cc) {
    // if cc is an existing cc
    if (exists(cc)) return cc_bitpos_.at(cc);
    // then try to find an available pos in recycle bin
    if (!recycle_bin_.empty()) {
        int pos = recycle_bin_.top();
        recycle_bin_.pop();
        cc_bitpos_[cc] = pos;
        return pos;
    }
    // otherwise realloc new space
    int pos = bits_.size();
    bits_.resize(pos + units_per_counter_, 0);
    cc_bitpos_[cc] = pos;
    return pos;
}

void DynDGraphMgr::delCC(const int cc) {
    int pos = cc_bitpos_.at(cc);
    // clean range [start, end)
    auto start = bits_.begin();
    std::advance(start, pos);
    auto end = start;
    std::advance(end, units_per_counter_);
    std::fill(start, end, 0);
    // recycling
    recycle_bin_.push(pos);
    cc_bitpos_.erase(cc);
}

void DynDGraphMgr::reverseFanOut(const dir::DGraph& G, const int cv,
                                 std::unordered_set<int>& modified) {
    if (!G.isNode(cv)) return;
    const auto& nd = G[cv];
    for (auto ni = nd.beginInNbr(); ni != nd.endInNbr(); ni++) {
        int cu = nd.getNbrID(ni);
        if (!isGEq(cu, cv)) {
            mergeCC(cu, cv);
            modified.insert(cu);
        }
    }
}

void DynDGraphMgr::addEdge(const int u, const int v) {
    int cu = getCC(u), cv = getCC(v);
    // omit self-loop edges and edges already in DAG
    if (cu != cv && !dag_.isEdge(cu, cv)) {
        dag_.addEdge(cu, cv);
        if (!exists(cu) || !exists(cv) || !isGEq(cu, cv)) {
            new_cc_edges_.emplace_back(cu, cv);
        }
    }
}

std::vector<int> DynDGraphMgr::getAffectedNodes() {
    if (new_cc_edges_.empty()) return std::vector<int>();
    SCCVisitor<dir::DGraph> dfs(dag_);
    dfs.performDFS();

    // new CCs in topological order
    auto new_ccs = dfs.getCCSorted();

    // CC connections
    auto cc_edges = dfs.getCCEdges();

    // re-arrange current CCs
    std::unordered_map<int, int> co_cn;   // old-CC -> new-CC mapping
    std::vector<std::vector<int>> cc_cc;  // {<new_cc, old_cc1, old_cc2, ...>}
    int cc = -1;
    for (auto& pr : dfs.getCNEdges()) {
        int cn = pr.first, co = pr.second;
        co_cn[co] = cn;
        if (cc != cn) {
            cc = cn;
            cc_cc.push_back({cn});
        }
        if (co != cn) cc_cc.back().push_back(co);
    }

    // generate or merge bits in each CC
    std::unordered_set<int> modified;  // record possibly modified CCs
    for (auto& ccs : cc_cc) {
        int c0 = ccs[0];
        // if the CC is newly created, then generate bits for it
        if (!exists(c0)) {
            genCounter(c0);
            modified.insert(c0);
        }
        for (int i = 1; i < ccs.size(); i++) {
            int ci = ccs[i];
            if (!exists(ci)) genCounter(ci);
            mergeCC(c0, ci);
            modified.insert(c0);
            delCC(ci);
        }
    }

    // also need to check new edges between two CCs
    dir::DGraph sub_dag;
    for (auto& edge : new_cc_edges_) {
        int ci = co_cn.at(edge.first), cj = co_cn.at(edge.second);
        if (ci != cj && modified.find(cj) == modified.end())
            sub_dag.addEdge(ci, cj);
    }
    new_cc_edges_.clear();

    // update DAG
    dag_.clear();
    dag_.addNodes(new_ccs);
    for (auto& pr : cc_edges) dag_.addEdge(pr.first, pr.second);

    // update CC bits in topological order
    for (auto it = new_ccs.rbegin(); it != new_ccs.rend(); it++) {
        int cj = *it;
        if (modified.find(cj) != modified.end()) {
            // if cj is modified, then do reverse fan-out
            reverseFanOut(dag_, cj, modified);
        } else {
            // else if an old CC has new in-coming edges
            reverseFanOut(sub_dag, cj, modified);
        }
    }

    // update node-cc mapping
    std::vector<int> affected_nodes;
    for (auto& pr : nd_cc_) {
        // use ref: because we will update cc of nd later
        int nd = pr.first, &cc = pr.second;
        cc = co_cn.at(cc);
        if (modified.find(cc) != modified.end()) {
            affected_nodes.push_back(nd);
        }
    }
    return affected_nodes;
}

void DynDGraphMgr::clear(const bool deep) {
    new_cc_edges_.clear();
    InputMgr::clear(deep);
    if (deep) {
        HyperANF::clear();
        dag_.clear();
        while (!recycle_bin_.empty()) recycle_bin_.pop();
    }
}
