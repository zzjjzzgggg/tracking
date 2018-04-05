/**
 * Copyright (C) by J.Z. (04/05/2018 11:42)
 * Distributed under terms of the MIT license.
 */

#include "sieve_adn.h"

void SieveADN::delTheta(const int i) {
    int pos = thi_pos_[i];
    S_buf_[pos].clear();
    recycle_bin_.push(pos);
    thi_pos_.erase(i);
}

void SieveADN::addTheta(const int i) {
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

bool SieveADN::updateMaxGain(const std::vector<int>& nodes) {
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

void SieveADN::updateThresholds() {
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

void SieveADN::sieve(const std::vector<int>& nodes) {
    for (int u : nodes) {
        for (auto& pr : thi_pos_) {
            int i = pr.first;
            auto& S = getS(i);
            if (S.find(u) == S.end() && S.size() < budget_) {
                double threshold = getThreshold(i),
                       gain = input_mgr_.getGain(u, S);
                if (gain >= threshold) S.insert(u);
            }
        }
    }
}

void SieveADN::processBatch(std::vector<std::pair<int, int>>& edges) {
    input_mgr_.addEdges(edges);
    auto nodes = input_mgr_.getAffectedNodes();
    if (nodes.empty()) return;

    // update maximum gain
    bool is_changed = updateMaxGain(nodes);

    // update thresholds if maximum gain changes
    if (is_changed) updateThresholds();

    // sieve
    sieve(nodes);
}

double SieveADN::getResult() const {
    int i_mx = -100;
    double rwd_mx =0;
    for (auto& pr: thi_pos_) {
        int i = pr.first;
        const auto& S = getS(i);
        double rwd = input_mgr_.getReward(S);
        if (rwd > rwd_mx) {
            rwd_mx = rwd;
            i_mx = i;
        }
    }
    printf("i: %d\n", i_mx);
    return rwd_mx;
}
