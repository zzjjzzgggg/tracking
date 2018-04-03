/**
 * Copyright (C) by J.Z. (04/03/2018 10:59)
 * Distributed under terms of the MIT license.
 */

#ifndef __ADN_MGR_H__
#define __ADN_MGR_H__

#include "stdafx.h"

class ADNMgr: public HyperANF {
private:
    DynDGraph DAG_;

private:
    /**
     * Check whether node u belongs to a CC; otherwise create a CC as u itself.
     */
    int checkCC(const int u) {
        if (nd_cc_.find(u) != nd_cc_.end())
            return nd_cc_.at(u);
        else
            nd_cc_[u] = u;
        return u;
    }

public:
    ADNMgr(const int p=16): HyperANF(p) {}

    void addEdge(const int u, const int v);
    void addEdges(const std::vector<std::pair<int, int>>& edges);

    /**
     * Run DFS to identify CCs in DAG
     */
    void updateDAG();

    std::vector<int> getAffectedNodes();


}; /* ADNMgr */

#endif /* __ADN_MGR_H__ */
