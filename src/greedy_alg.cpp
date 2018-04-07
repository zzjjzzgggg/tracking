/**
 * Copyright (C) by J.Z. (04/06/2018 09:19)
 * Distributed under terms of the MIT license.
 */

#include "greedy_alg.h"

double GreedyAlg::run() {
    auto cmp = [](Elem& a, Elem& b) { return a.gain < b.gain; };
    std::priority_queue<Elem, std::vector<Elem>, decltype(cmp)> pq(cmp);

    for (int node : input_ptr_->getNodesAll()) pq.emplace(node);

    std::vector<int> chosen;
    chosen.reserve(budget_);

    osutils::Timer tm;

    double rwd = 0;

    int seq = 1;
    while (!pq.empty() && seq <= budget_) {
        Elem e = pq.top();
        pq.pop();
        if (e.seq == seq) {
            e.time = tm.seconds();
            chosen.push_back(e.node);
            seq++;
            rwd += e.gain;
        } else {
            e.gain = input_ptr_->getGain(e.node, chosen);
            e.seq = seq;
            pq.push(std::move(e));
        }
    }

    return rwd;
}
