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
        int seq, node;      // seq: the round number that gain is updated
        double gain, time;  // chosen time in seconds

    public:
        Elem() : node(-1), seq(-1), gain(-1) {}
        Elem(int v, int s = 0, double g = std::numeric_limits<double>::max())
            : node(v), seq(s), gain(g) {}

        void echo() const {
            printf("i:%3d g:%.2e t:%.1f\n", seq, gain, time);
        }
    };

private:
    const InputMgr* input_ptr_;
    int budget_;

public:
    GreedyAlg(const InputMgr* input, const int budget)
        : input_ptr_(input), budget_(budget) {}

    /**
     * Run Greedy Algorithm on data provided by InputMgr.
     * Return reward.
     */
    double run();

}; /* GreedyAlg */

#endif /* __GREEDY_ALG_H__ */
