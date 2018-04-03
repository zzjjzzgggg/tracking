/**
 * Copyright (C) by J.Z. (04/03/2018 14:43)
 * Distributed under terms of the MIT license.
 */

#include "adn_mgr.h"

void ADNMgr::addEdge(const int u, const int v) {
    int cu = checkCC(u), cv = checkCC(v);
    // omit selfloop edges and edges already in DAG
    if (cu != cv && !DAG_.isEdge(cu, cv)) DAG_.addEdge(cu, cv);
}

void ADNMgr::addEdges(const std::vector<std::pair<int, int>>& edges) {
    for (auto& edge : edges) addEdge(edge.first, edge.second);
}

void ADNMgr::updateDAG() {
    SCCVisitor<DynDGraph> dfs(DAG_);
    dfs.performDFS();
    // dfs.getCCEdges();
    // dfs.getCNEdges();
    // dfs.getCCSorted();
}
