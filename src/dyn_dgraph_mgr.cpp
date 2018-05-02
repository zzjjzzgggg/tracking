/**
 * Copyright (C) by J.Z. (04/03/2018 14:43)
 * Distributed under terms of the MIT license.
 */
#include "dyn_dgraph_mgr.h"

std::vector<int> DynDGraphMgr::getNodes() const {
    std::vector<int> nodes;
    for (auto& pr : nd_cc_) nodes.push_back(pr.first);
    return nodes;
}

int DynDGraphMgr::getCC(const int u) {
    if (nd_cc_.find(u) != nd_cc_.end())
        return nd_cc_.at(u);
    else
        nd_cc_[u] = u;
    return u;
}

int DynDGraphMgr::getPos(const int cc) {
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

void DynDGraphMgr::deleteCC(const int cc) {
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
        mergeCounter(cc_bitpos_[cu], cc_bitpos_[cv]);
        modified.insert(cu);
    }
}

void DynDGraphMgr::addEdge(const int u, const int v) {
    int cu = getCC(u), cv = getCC(v);
    // omit self-loop edges and edges already in DAG
    if (cu != cv && !dag_.isEdge(cu, cv)) {
        dag_.addEdge(cu, cv);
        new_cc_edges_.emplace_back(cu, cv);
    }
}

std::vector<int> DynDGraphMgr::getAffectedNodes() {
    if (new_cc_edges_.empty()) return std::vector<int>();
    SCCVisitor<dir::DGraph> dfs(dag_);
    dfs.performDFS();

    // CCs in topological order
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
        if (cc_bitpos_.find(c0) == cc_bitpos_.end()) {
            genHLLCounter(getPos(c0));
            modified.insert(c0);
        }
        for (int i = 1; i < ccs.size(); i++) {
            int ci = ccs[i];
            if (cc_bitpos_.find(ci) == cc_bitpos_.end())
                genHLLCounter(getPos(ci));
            int pos = getPos(ci);
            mergeCounter(getPos(c0), pos);
            modified.insert(c0);
            deleteCC(ci);
        }
    }

    // also need to check new edges between two CCs
    dir::DGraph sub_dag;
    for (auto& edge : new_cc_edges_) {
        int ci = co_cn.at(edge.first), cj = co_cn.at(edge.second);
        if (ci != cj && modified.find(cj) == modified.end() &&
            !sub_dag.isEdge(ci, cj))
            sub_dag.addEdge(ci, cj);
    }
    new_cc_edges_.clear();

    // update DAG
    dag_.clear();
    for (auto& pr : cc_edges) dag_.addEdge(pr.first, pr.second);

    // update CC bits in topological order
    for (auto it = new_ccs.rbegin(); it != new_ccs.rend(); it++) {
        int cj = *it;
        if (modified.find(cj) != modified.end()) {  // if cj is modified
            reverseFanOut(dag_, cj, modified);      // then do reverse fan-out
        } else {  // if an old CC has new in-coming edges
            reverseFanOut(sub_dag, cj, modified);
        }
    }

    // update node-cc mapping
    // TODO: improve efficiency
    std::vector<int> affected_nodes;
    for (auto& pr : nd_cc_) {
        int nd = pr.first, &cc = pr.second;
        // NOTE: If a CC is singleton in previous step, then after updating the
        // DAG in Line 130, this CC will disappear in current step.
        if (co_cn.find(cc) != co_cn.end()) {
            cc = co_cn.at(cc);
            if (modified.find(cc) != modified.end())
                affected_nodes.push_back(nd);
        }
    }

    // printf("affected nodes: %lu\n", affected_nodes.size());

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

double DynDGraphMgr::getReward(const int node) const { return estimate(node); }
double DynDGraphMgr::getReward(const std::vector<int>& S) const {
    return estimate(S.begin(), S.end());
}
double DynDGraphMgr::getReward(const std::unordered_set<int>& S) const {
    return estimate(S.begin(), S.end());
}
double DynDGraphMgr::getGain(const int u, const std::vector<int>& S,
                             const bool check) const {
    return getGain(u, S.begin(), S.end(), check);
}
double DynDGraphMgr::getGain(const int u, const std::unordered_set<int>& S,
                             const bool check) const {
    return getGain(u, S.begin(), S.end(), check);
}
