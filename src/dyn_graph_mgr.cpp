/**
 * Copyright (C) by J.Z. (04/25/2018 08:48)
 * Distributed under terms of the MIT license.
 */

#include "dyn_graph_mgr.h"

void DynGraphMgr::addEdge(const int u, const int v) {
    if (!graph_.isEdge(u, v)) {
        graph_.addEdge(u, v);
        affected_nodes_.insert(u);

        // get out-neighbors of node v
        std::vector<int> v_to, to_u;
        v_to.push_back(v);
        if (graph_.isNodeL(v)) {
            const auto& tmp = graph_.getNodeL(v).getNbrs();
            v_to.insert(v_to.end(), tmp.begin(), tmp.end());
        }
        to_u.push_back(u);
        if (graph_.isNodeR(u)) {
            const auto& tmp = graph_.getNodeR(u).getNbrs();
            to_u.insert(to_u.end(), tmp.begin(), tmp.end());
        }

        // connect the two parts
        for (int x : to_u) {
            auto& node_x = graph_.getNodeL(x);
            for (int y : v_to) {
                if (x != y && !node_x.isNbr(y)) {
                    graph_.addEdge(x, y);
                    affected_nodes_.insert(x);
                }
            }
        }
    }
}

void DynGraphMgr::clear(const bool deep) {
    InputMgr::clear(deep);
    if (deep) graph_.clear();
}

std::vector<int> DynGraphMgr::getNodes() const {
    std::vector<int> nodes;
    nodes.reserve(graph_.getNodesL());
    for (auto it = graph_.beginNIL(); it != graph_.endNIL(); ++it)
        nodes.push_back(it->first);
    return nodes;
}
