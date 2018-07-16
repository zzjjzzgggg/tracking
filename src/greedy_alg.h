/**
 * Copyright (C) by J.Z. (04/06/2018 09:14)
 * Distributed under terms of the MIT license.
 */

#ifndef __GREEDY_ALG_H__
#define __GREEDY_ALG_H__

#include "input_mgr.h"

class GreedyAlg {
private:
    class Elem {
    public:
        int seq, node;  // seq: the round number that gain is updated
        double gain;

    public:
        Elem() : node(-1), seq(-1), gain(-1) {}
        Elem(int v, int s = 0, double g = std::numeric_limits<double>::max())
            : node(v), seq(s), gain(g) {}

        void echo() const { printf("i:%3d g:%.2e\n", seq, gain); }
    };

private:
    const InputMgr* input_ptr_;
    int budget_;

    std::vector<int> chosen;

public:
    GreedyAlg(const InputMgr* input, const int budget)
        : input_ptr_(input), budget_(budget) {
        chosen.reserve(budget_);
    }

    /**
     * Run Greedy Algorithm on data provided by InputMgr.
     * Return reward.
     */
    double run() {
        auto cmp = [](Elem& a, Elem& b) { return a.gain < b.gain; };
        std::priority_queue<Elem, std::vector<Elem>, decltype(cmp)> pq(cmp);

        for (int node : input_ptr_->getNodes()) pq.emplace(node);

        double rwd = 0;
        int seq = 1;
        while (!pq.empty() && seq <= budget_) {
            Elem e = pq.top();
            pq.pop();
            if (e.seq == seq) {
                chosen.push_back(e.node);
                seq++;
                rwd += e.gain;
                // printf("[%d]: %.4f\n", e.node, e.gain);
            } else {
                e.gain = input_ptr_->getGain(e.node, chosen);
                e.seq = seq;
                pq.push(std::move(e));
            }
        }
        return rwd;
    }

    int getOracleCalls() const { return input_ptr_->getOracleCalls(); }

    void saveSolution(const std::string& filename) const {
        ioutils::saveVec(chosen, filename);
    }

}; /* GreedyAlg */

#endif /* __GREEDY_ALG_H__ */
