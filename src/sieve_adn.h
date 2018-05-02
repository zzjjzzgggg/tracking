/**
 * Copyright (C) by J.Z. (04/05/2018 10:47)
 * Distributed under terms of the MIT license.
 */

#ifndef __SIEVE_ADN_H__
#define __SIEVE_ADN_H__

#include "stdafx.h"

template <class InputMgr>
class SieveADN {
private:
    int budget_;
    double eps_, gain_mx_ = 0;

    InputMgr input_mgr_;

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

public:
    SieveADN(const int budget, const double eps) : budget_(budget), eps_(eps) {
        // |\Theta| = O(\epsilon^{-1}\log 2k)
        S_buf_.reserve((int)(std::log2(2 * budget_) / eps_));
    }

    /**
     * Copy constructor.
     */
    SieveADN(const SieveADN& o)
        : budget_(o.budget_), eps_(o.eps_), gain_mx_(o.gain_mx_),
          input_mgr_(o.input_mgr_), S_buf_(o.S_buf_), thi_pos_(o.thi_pos_),
          recycle_bin_(o.recycle_bin_) {}

    /**
     * Copy assignment.
     */
    SieveADN& operator=(const SieveADN& o) {
        budget_ = o.budget_;
        eps_ = o.eps_;
        gain_mx_ = o.gain_mx_;
        input_mgr_ = o.input_mgr_;
        S_buf_ = o.S_buf_;
        thi_pos_ = o.thi_pos_;
        recycle_bin_ = o.recycle_bin_;
        return *this;
    }

    void feedEdge(const int u, const int v) { input_mgr_.addEdge(u, v); }
    void feedEdges(const IntPrV& edges) { input_mgr_.addEdges(edges); }

    // update and return num of affected nodes
    int update(const bool check = true);
    int getOracleCalls() const { return input_mgr_.getOracleCalls(); }

    /**
     * Get current maximum reward
     */
    std::pair<int, double> getResult() const;

    std::vector<int> getSolution(const int i) const {
        const auto& S = getS(i);
        return std::vector<int>(S.begin(), S.end());
    }

    InputMgr& getInputMgr() { return input_mgr_; }

    int getNumThresholds() const { return thi_pos_.size(); }

    /**
     * If deep = true, then clean everything, including input_mgr_ and maintaned
     * graph; otherwise, only clean temporal results maintaned in input_mgr_.
     */
    void clear(const bool deep = false);

}; /* SieveADN */

// implementations
template <class InputMgr>
void SieveADN<InputMgr>::delTheta(const int i) {
    int pos = thi_pos_[i];
    S_buf_[pos].clear();
    recycle_bin_.push(pos);
    thi_pos_.erase(i);
}

template <class InputMgr>
void SieveADN<InputMgr>::addTheta(const int i) {
    int pos;
    if (!recycle_bin_.empty()) {  // if have available room
        pos = recycle_bin_.top();
        recycle_bin_.pop();
        S_buf_[pos].clear();  // make sure it is clean
    } else {                  // otherwise realloc room
        pos = S_buf_.size();
        S_buf_.push_back(std::unordered_set<int>());
    }
    thi_pos_[i] = pos;
}

template <class InputMgr>
bool SieveADN<InputMgr>::updateMaxGain(const std::vector<int>& nodes) {
    bool is_changed = false;
    for (int u : nodes) {
        double rwd = input_mgr_.getReward(u);
        if (rwd > gain_mx_) {
            gain_mx_ = rwd;
            is_changed = true;
        }
    }
    return is_changed;
}

template <class InputMgr>
void SieveADN<InputMgr>::updateThresholds() {
    int new_li = (int)std::floor(std::log((1 - eps_) * gain_mx_) /
                                 std::log(1 + eps_)),
        new_ui = (int)std::ceil(std::log(2 * budget_ * gain_mx_) /
                                std::log(1 + eps_));
    int li, ui;               // lower bound and upper bound of theata index
    if (!thi_pos_.empty()) {  // delete outdated thresholds
        li = thi_pos_.begin()->first;
        ui = thi_pos_.rbegin()->first;
        while (li <= ui && li < new_li) {
            delTheta(li);
            li++;
        }
    }
    li = thi_pos_.empty() ? new_li : ui + 1;
    for (int i = li; i <= new_ui; i++) addTheta(i);
}

template <class InputMgr>
int SieveADN<InputMgr>::update(const bool check) {
    auto nodes = input_mgr_.getAffectedNodes();
    if (nodes.empty()) return 0;

    // update maximum gain and thresholds
    if (updateMaxGain(nodes)) updateThresholds();

    // sieve
    for (int u : nodes) {
        for (auto& pr : thi_pos_) {
            int i = pr.first;
            auto& S = getS(i);
            if (S.find(u) == S.end() && S.size() < budget_) {
                double threshold = getThreshold(i),
                       gain = input_mgr_.getGain(u, S, check);
                if (gain >= threshold) S.insert(u);
            }
        }
    }
    return nodes.size();
}

template <class InputMgr>
std::pair<int, double> SieveADN<InputMgr>::getResult() const {
    int i_mx = -100;
    double rwd_mx = 0;
    for (auto& pr : thi_pos_) {
        int i = pr.first;
        double rwd = input_mgr_.getReward(getS(i));
        if (rwd > rwd_mx) {
            rwd_mx = rwd;
            i_mx = i;
        }
    }
    return std::make_pair(i_mx, rwd_mx);
}

template <class InputMgr>
void SieveADN<InputMgr>::clear(const bool deep) {
    input_mgr_.clear(deep);
    if (deep) {
        gain_mx_ = 0;
        thi_pos_.clear();
        while (!recycle_bin_.empty()) recycle_bin_.pop();
        for (auto& S : S_buf_) S.clear();
    }
}

#endif /* __SIEVE_ADN_H__ */
