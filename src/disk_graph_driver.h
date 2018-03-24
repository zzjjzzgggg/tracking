#ifndef __DISK_WALK_DRIVER_H__
#define __DISK_WALK_DRIVER_H__

#include "stdafx.h"
#include "graph_driver.h"

class DiskGraphDriver : public GraphDriver {
private:
    int cur_src_node_ = -1;
    ioutils::LZ4In lzi_;

public:
    DiskGraphDriver(Config* conf) : GraphDriver(conf) {
        lzi_.open(conf_->graph_fnm.c_str());
    }

    void reset() override {
        cur_src_node_ = -1;
        lzi_.open(conf_->graph_fnm.c_str());
    }

    NodeContext getNodeContext(const int node) override {
        NodeContext context{node};
        int cur_dst_node;
        while (cur_src_node_ < node && (!lzi_.eof())) {
            lzi_.load(cur_src_node_);
            lzi_.load(cur_dst_node);
        }
        while (cur_src_node_ == node && (!lzi_.eof())) {
            context.addNbr(cur_dst_node);
            lzi_.load(cur_src_node_);
            lzi_.load(cur_dst_node);
        }
        if (ChosenSet::isConnected(node)) {
            double weight = conf_->getAbsbWt(context.size());
            context.addNbr(conf_->absb_node, weight);
        }
        return context;
    }
};

#endif
