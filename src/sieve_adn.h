/**
 * Copyright (C) by J.Z. (04/05/2018 10:47)
 * Distributed under terms of the MIT license.
 */

#ifndef __SIEVE_ADN_H__
#define __SIEVE_ADN_H__

#include "input_mgr.h"

class SieveADN {
private:
    InputMgr& input_mgr_;

    int budget_;
    double eps_, gain_mx_ = 0;

    std::vector<std::unordered_set<int>> S_buf_;
    std::map<int, int> thi_pos_;  // theta_index --> set_position

    std::stack<int> recycle_bin_;

private:
    /**
     * theta = (1+\epsilon)^i / (2k)
     */
    inline double getThreshold(const int i) const {
        return std::pow(1 + eps_, i) / (2 * budget_);
    }

    void delTheta(const int i);

    /**
     * Given a threshold index i, return set S.
     */
    inline std::unordered_set<int>& getS(const int i) {
        return S_buf_[thi_pos_.at(i)];
    }
    inline const std::unordered_set<int>& getS(const int i) const {
        return S_buf_[thi_pos_.at(i)];
    }

    /**
     * Add a new threshold with index i.
     */
    void addTheta(const int i);
    bool updateMaxGain(const std::vector<int>& nodes);
    void updateThresholds();
    void sieve(const std::vector<int>& nodes);

public:
    SieveADN(InputMgr& input, const int budget, const double eps)
        : input_mgr_(input), budget_(budget), eps_(eps) {
        // |\Theta| = O(\epsilon^{-1}\log 2k)
        S_buf_.reserve((int)(std::log2(2 * budget_) / eps_));
    }

    /**
     * Process a batch of edges.
     */
    void processBatch(std::vector<std::pair<int, int>>& edges);

    /**
     * Get current maximum reward
     */
    double getResult() const;

}; /* SieveADN */

#endif /* __SIEVE_ADN_H__ */
