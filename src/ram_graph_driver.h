#ifndef __RAM_GRAPH_DRIVER_H__
#define __RAM_GRAPH_DRIVER_H__

#include "stdafx.h"
#include "graph_driver.h"

class RAMGraphDriver : public GraphDriver {
public:
    graph::DGraph G;

public:
    RAMGraphDriver(Config* conf, const int core_id = 0) : GraphDriver(conf) {
        string filename = conf_->n_cpus == 1
                              ? conf_->graph_fnm
                              : fmt::format(conf_->graph_fnm, core_id);
        G = graph::loadBinEdgeList<graph::DGraph>(filename);
    }

    NodeContext getNodeContext(const int node) override {
        NodeContext ctxt{node};
        int deg = getOutDeg(node);
        auto& node_obj = G.getNode(node);
        for (int d = 0; d < deg; d++) ctxt.addNbr(node_obj.getOutNbr(d));
        if (ChosenSet::isConnected(node))
            ctxt.addNbr(conf_->absb_node, conf_->getAbsbWt(ctxt.size()));
        return ctxt;
    }

    int getOutDeg(const int node) const { return G.getNode(node).getOutDeg(); }
};

#endif
