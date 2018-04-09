/**
 * Copyright (C) by J.Z. (04/09/2018 16:46)
 * Distributed under terms of the MIT license.
 */

#ifndef __EVAL_STREAM_H__
#define __EVAL_STREAM_H__

#include "stdafx.h"

template <class InputMgr>
class EvalStream {
private:
    int L_, cur_ = 0;
    std::vector<std::vector<IntPr>> edge_buf_;

    InputMgr input_mgr_;

public:
    EvalStream(const int L) : L_(L) { edge_buf_.resize(L); }

    void addEdge(const int u, const int v, const int l) {
        edge_buf_[(cur_ + l - 1) % L_].emplace_back(u, v);
    }

    void clear() {
        edge_buf_[cur_].clear();
        cur_ = (cur_ + 1) % L_;
    }

    double eval(const std::vector<int>& nodes) {
        input_mgr_.clear(true);
        for (auto& edges : edge_buf_)
            if (!edges.empty()) input_mgr_.addEdges(edges);
        return input_mgr_.getReward(nodes);
    }

    InputMgr& getInputMgr() {
        input_mgr_.clear(true);
        for (auto& edges : edge_buf_)
            if (!edges.empty()) input_mgr_.addEdges(edges);
        return input_mgr_;
    }

}; /* EvalStream */

#endif /* __EVAL_STREAM_H__ */
