#ifndef __GRAPH_MANAGER_H__
#define __GRAPH_MANAGER_H__

#include "stdafx.h"
#include "ram_graph_driver.h"

class GraphManager {
private:
    Config* conf_;
    vector<RAMGraphDriver*> drivers_;

public:
    GraphManager(Config* conf) : conf_(conf) { drivers_.reserve(conf->n_cpus); }

    void add(RAMGraphDriver* driver) { drivers_.push_back(driver); }

    NodeContext getNodeContext(const int node) {
        int core = node / conf_->n_nodes_a_cpu;
        return drivers_[core]->getNodeContext(node);
    }

    vector<int> getInNbrs(const int node) {
        vector<int> v;
        for (auto driver : drivers_) {
            if (driver->G.isNode(node)) {
                auto&& node_obj = driver->G.getNode(node);
                for (int d = 0; d < node_obj.getInDeg(); d++)
                    v.push_back(node_obj.getInNbr(d));
            }
        }
        return v;
    }

    int getOutDeg(const int node) const {
        int core = node / conf_->n_nodes_a_cpu;
        return drivers_[core]->getOutDeg(node);
    }

    double P(const int i, const int j = 0) const {
        int deg = getOutDeg(i);
        double wt = ChosenSet::isConnected(i) ? conf_->getAbsbWt(deg) : 0;
        return j >= 0 ? 1 / (deg + wt) : wt / (deg + wt);
    }
};

#endif /* __GRAPH_MANAGER_H__ */
