/**
 * Copyright (C) by J.Z. (04/03/2018 14:43)
 * Distributed under terms of the MIT license.
 */

#include "dyngraph_mgr.h"

int DynGraphMgr::getCC(const int u) {
    if (nd_cc_.find(u) != nd_cc_.end())
        return nd_cc_.at(u);
    else
        nd_cc_[u] = u;
    return u;
}

int DynGraphMgr::getPos(const int cc) {
    // if cc is an existing cc
    if (cc_bitpos_.find(cc) != cc_bitpos_.end()) return cc_bitpos_.at(cc);

    // then try to find an available pos in recycle bin
    if (!recycle_bin_.empty()) {
        int pos = recycle_bin_.top();
        recycle_bin_.pop();
        cc_bitpos_[cc] = pos;  // register this CC in CC-pos mapping
        return pos;
    }

    // otherwise realloc new space
    int pos = bits_.size();
    bits_.resize(pos + units_per_counter_, 0);
    cc_bitpos_[cc] = pos;
    return pos;
}

void DynGraphMgr::freePos(const int pos) {
    // clean range [start, end)
    auto start = bits_.begin();
    std::advance(start, pos);
    auto end = start;
    std::advance(end, units_per_counter_);
    std::fill(start, end, 0);
    // recycling
    recycle_bin_.push(pos);
    cc_bitpos_.erase(pos);
}

void DynGraphMgr::reverseFanOut(const dir::DGraph& G, const int cv,
                                std::unordered_set<int>& modified) {
    if (!G.isNode(cv)) return;
    const auto& nd = G[cv];
    for (auto ni = nd.beginInNbr(); ni != nd.endInNbr(); ni++) {
        int cu = nd.getNbrID(ni);
        mergeCounter(cc_bitpos_[cu], cc_bitpos_[cv]);
        modified.insert(cu);
    }
}

void DynGraphMgr::addEdge(const int u, const int v) {
    int cu = getCC(u), cv = getCC(v);
    // omit self-loop edges and edges already in DAG
    if (cu != cv && !DAG_.isEdge(cu, cv)) {
        DAG_.addEdge(cu, cv);
        new_cc_edges_.emplace_back(cu, cv);
    }
}

void DynGraphMgr::addEdges(const std::vector<std::pair<int, int>>& edges) {
    // new_cc_edges_.clear();
    for (auto& edge : edges) addEdge(edge.first, edge.second);
}

std::vector<int> DynGraphMgr::updateDAG() {
    SCCVisitor<dir::DGraph> dfs(DAG_);
    dfs.performDFS();

    // CCs in topological order
    auto cc_new_vec = dfs.getCCSorted();

    // CC connections
    auto cc_edges_vec = dfs.getCCEdges();

    // re-arrange current CCs
    std::unordered_map<int, int> cco_ccn;  // old-CC -> new-CC mapping
    std::vector<std::vector<int>> cc_cc;
    int cc = -1;
    for (auto& pr : dfs.getCNEdges()) {
        int ccn = pr.first, cco = pr.second;
        cco_ccn[cco] = ccn;
        if (cc != ccn) {
            cc = ccn;
            cc_cc.push_back({ccn});
        }
        if (cco != ccn) cc_cc.back().push_back(cco);
    }

    // generate or merge bits in each CC
    std::unordered_set<int> modified;  // record possibly modified CCs
    for (auto& ccs : cc_cc) {
        int pos0 = getPos(ccs[0]);
        // if the CC is newly created, then generate bits for it
        if (cc_.find(ccs[0]) == cc_.end()) {
            genHLLCounter(pos0);
            modified.insert(ccs[0]);
        }

        for (int i = 1; i < ccs.size(); i++) {
            int pos1 = getPos(ccs[i]);
            if (cc_.find(ccs[i]) == cc_.end()) genHLLCounter(pos1);
            mergeCounter(pos0, pos1);
            modified.insert(ccs[0]);
            freePos(pos1);
        }
    }

    // also need to check new edges between CCs
    dir::DGraph sub_DAG;
    for (auto& edge : new_cc_edges_) {
        int cci = cco_ccn.at(edge.first), ccj = cco_ccn.at(edge.second);
        if (cci != ccj || modified.find(ccj) == modified.end())
            sub_DAG.addEdge(cci, ccj);
    }

    // update DAG
    DAG_.clear();
    for (auto& pr : cc_edges_vec) DAG_.addEdge(pr.first, pr.second);

    // update CC bits in topological order
    for (auto it = cc_new_vec.rbegin(); it != cc_new_vec.rend(); it++) {
        int ccj = *it;
        if (modified.find(ccj) != modified.end()) {
            // if ccj is modified, then do reverse fan-out
            reverseFanOut(DAG_, ccj, modified);
        } else {
            // if an old CC has new in-coming edges
            reverseFanOut(sub_DAG, ccj, modified);
        }
    }

    // update CCs
    cc_.clear();
    cc_.insert(cc_new_vec.begin(), cc_new_vec.end());

    // update node-cc mapping
    std::vector<int> affected_nodes;
    for (auto& pr : nd_cc_) {
        int nd = pr.first, &cc = pr.second;
        cc = cco_ccn.at(cc);
        if (modified.find(cc) != modified.end()) affected_nodes.push_back(nd);
    }
    return affected_nodes;
}
