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
    std::vector<IntPrV> edge_buf_;

public:
    EvalStream(const int L) : L_(L) { edge_buf_.resize(L); }

    void addEdge(const int u, const int v, const int l = 1) {
        edge_buf_[(cur_ + l - 1) % L_].emplace_back(u, v);
    }

    void next() {
        edge_buf_[cur_].clear();
        cur_ = (cur_ + 1) % L_;
    }

    const InputMgr getInputMgr() {
        InputMgr input_mgr;
        for (auto& edges : edge_buf_) input_mgr.addEdges(edges);
        input_mgr.getAffectedNodes();
        return input_mgr;
    }

    void saveEdges(const std::string& filename) const {
        auto wt_ptr = ioutils::getIOOut(filename);
        for (auto& edges : edge_buf_) {
            for (auto& edge : edges)
                wt_ptr->save("{}\t{}\n"_format(edge.first, edge.second));
        }
        printf("saved to %s\n", filename.c_str());
    }

    void debug() const { saveEdges("eval.edges"); }

}; /* EvalStream */

#endif /* __EVAL_STREAM_H__ */
