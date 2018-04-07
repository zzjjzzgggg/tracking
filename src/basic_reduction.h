/**
 * Copyright (C) by J.Z. (04/06/2018 15:51)
 * Distributed under terms of the MIT license.
 */

#ifndef __BASIC_REDUCTION_H__
#define __BASIC_REDUCTION_H__

#include "sieve_adn.h"

template <class InputMgr>
class BasicReduction {
private:
    std::vector<SieveADN<InputMgr>*> sieve_ptrs_;
    int L_, cur_;  // cur_ points to the instance with l = 1

public:
    BasicReduction(const int L, const int budget, const double eps) : L_(L) {
        sieve_ptrs_.resize(L);
        for (int l = 0; l < L_; l++)
            sieve_ptrs_[l] = new SieveADN<InputMgr>(budget, eps);
    }

    virtual ~BasicReduction() {
        for (int l = 0; l < L_; l++) delete sieve_ptrs_[l];
    }

    /**
     * Edge with lifetime l will be fed to instances with l' <= l.
     * Alg. instances in range [cur_, cur_ + l - 1] will be updated.
     */
    void addEdge(const int u, const int v, const int l);

    /**
     * Update each SieveADN instance.
     * Return the output of first instance, i.e., the one pointed by cur_.
     */
    double update();

    /**
     * Clear and prepare for next time step.
     * Conduct deep clean for the InputMgr pointed by cur_, light clean for
     * other InputMgr.
     */
    void clear();

    /**
     * Provided to Greedy Alg.
     */
    const InputMgr& getCurInputMgr() const {
        return sieve_ptrs_[cur_]->getInputMgr();
    }

}; /* BasicReduction */

// implementations
template <class InputMgr>
void BasicReduction<InputMgr>::addEdge(const int u, const int v, const int l) {
    int i = cur_;
    while (i != (cur_ + l - 1) % L_) {
        sieve_ptrs_[i]->addEdge(u, v);
        i = (i + 1) % L_;
    }
}

template <class InputMgr>
double BasicReduction<InputMgr>::update() {
    for (int i = 0; i < L_; ++i) {
        int l = (cur_ + i) % L_;
        sieve_ptrs_[l]->update();
    }
    return sieve_ptrs_[cur_]->getResult().second;
}

template <class InputMgr>
void BasicReduction<InputMgr>::clear() {
    sieve_ptrs_[cur_]->clear(true);
    for (int i = 0; i < L_; ++i) sieve_ptrs_[i]->clear();
    cur_ = (cur_ + 1) % L_;
}

#endif /* __BASIC_REDUCTION_H__ */
