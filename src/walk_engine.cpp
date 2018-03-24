#include "walk_engine.h"

void WalkEngine::initWalks() {
    int node_start = FIRST_NODE_IN_BUCKET(bkt_offset_);
    int node_end = std::min(conf_->n_nodes, node_start + conf_->n_nodes_a_cpu);

    for (int node = node_start; node < node_end; node++) {
        if (node == conf_->absb_node) continue;
        for (int id = 0; id < conf_->walks; id++) {
            long walk = Encoder::encode(GET_OFFSET(node), node, id);
            buf_odd_[GET_BUCKET(node) - bkt_offset_].push_back(walk);
        }
    }
}

void WalkEngine::startWalking() {
    while (true) {
        size_t non_empty_bkts = buf_odd_.size();
        driver_->reset();
        NodeContext context;
        for (size_t bkt = 0; bkt < buf_odd_.size(); bkt++) {
            if (buf_odd_[bkt].empty()) {
                non_empty_bkts--;
                continue;
            }
            vector<long> walks = std::move(buf_odd_[bkt]);
            std::sort(walks.begin(), walks.end(), std::less<unsigned long>());
            for (long walk : walks) {
                int node = Encoder::getNode(bkt + bkt_offset_, walk);
                if (node != context.node_)
                    context = driver_->getNodeContext(node);
                dispatcher_->dispatch(walk, context.forward(), false);
            }
        }
        if (non_empty_bkts == 0) break;
    }
}

bool WalkEngine::walkSync(const bool read_odd) {
    NodeContext context;
    vector<vector<long>>& frontiers = read_odd ? buf_odd_ : buf_eve_;
    int non_empty_bkts = frontiers.size();
    for (size_t bkt = 0; bkt < frontiers.size(); bkt++) {
        if (frontiers[bkt].empty()) {
            non_empty_bkts--;
            continue;
        }
        vector<long> walks = std::move(frontiers[bkt]);
        std::sort(walks.begin(), walks.end(), std::less<unsigned long>());
        for (long walk : walks) {
            int node = Encoder::getNode(bkt + bkt_offset_, walk);
            if (node != context.node_) context = driver_->getNodeContext(node);
            dispatcher_->dispatch(walk, context.forward(), read_odd);
        }
    }
    return non_empty_bkts == 0;
}

void WalkEngine::setWalksFrom(const int node, vector<long>&& walks) {
    assert(node != conf_->absb_node);
    buf_odd_[GET_BUCKET(node) - bkt_offset_] = std::move(walks);
}

void WalkEngine::registerDispatcher(const int bkt_offset,
                                    Dispatcher* dispatcher) {
    bkt_offset_ = bkt_offset;
    dispatcher_ = dispatcher;
    dispatcher_->bind(bkt_offset_, buf_odd_, buf_eve_);
}
