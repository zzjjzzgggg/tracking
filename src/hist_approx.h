/**
 * Copyright (C) by J.Z. (04/06/2018 22:45)
 * Distributed under terms of the MIT license.
 */

#ifndef __HIST_APPROX_H__
#define __HIST_APPROX_H__

#include "sieve_adn.h"

template <class InputMgr>
class HistApprox {
private:
    class Alg {
    public:
        int l_;  // this alg only process edges with lifetime no less than l_
        double val_;  // current solution value of this algorithm instance

        SieveADN<InputMgr>* sieve_ptr_;

    public:
        Alg(const int l, const int budget, const double eps) : l_(l), val_(0) {
            sieve_ptr_ = new SieveADN<InputMgr>(budget, eps);
        }
        virtual ~Alg() { delete sieve_ptr_; }

        /**
         * Copy constructor.
         */
        Alg(const Alg& alg) : l_(alg.l_), val_(alg.val_) {
            sieve_ptr_ = new SieveADN<InputMgr>(*alg.sieve_ptr_);
        }

    }; /* Alg */

    typedef typename std::list<Alg*>::const_iterator Iter;

private:
    int budget_;
    double eps_;

    std::list<Alg*> algs_;        // a list of alg instances
    std::map<int, Iter> l_iter_;  // lifetime - iterator mapping

private:
    /**
     * Create an Alg instance and place it at the correct position in the list.
     */
    void createAlgIfNecessary(const int l);

    /**
     * Remove epsion-reduction alg instances from the list according instances'
     * current output.
     */
    void removeRedundancy();

public:
    HistApprox(const int budget, const double eps)
        : budget_(budget), eps_(eps) {}
    virtual ~HistApprox() {
        for (auto it = algs_.begin(); it != algs_.end(); ++it) delete *it;
    }

    /**
     * Edge with lifetime l will be fed to instances with l' <= l.
     * Scan alg. instances in in the list from head to tail.
     */
    void addEdge(const int u, const int v, const int l);

    /**
     * Update each SieveADN instance.
     * Return the output of head instance.
     */
    double update();

    /**
     * Prepare for next time step.
     * Deleta the head instance if its l=1, and conduct light clean for other
     * instances.
     */
    void clear();

}; /* HistApprox */

// implementations
template <class InputMgr>
void HistApprox<InputMgr>::createAlgIfNecessary(const int l) {
    if (l_iter_.find(l) != l_iter_.end()) return;
    auto it = l_iter_.upper_bound(l);
    if (it == l_iter_.end())  // if l has no successor, then create a new
        algs_.insert(it, new Alg(l, budget_, eps_));
    else {  // else create a copy of its successor
        Alg* alg = new Alg(*(*it));
        // TODO process more edges
        algs_.insert(it, alg);
    }
}

template <class InputMgr>
void HistApprox<InputMgr>::removeRedundancy() {
    auto start = algs_.begin();
    while (start != algs_.end()) {
        double bound = *start->val_ * (1 - eps_);
        auto last = start, it = start;
        while (true) {
            ++it;
            if (it == algs_.end() || *it->val_ < bound) break;
            ++last;
        }
        if (start != last) {  // delete algs in range (start, last)
            it = start;
            while (++it != last) delete *it;
            algs_.erase(++start, last);
            start = last;
        } else
            ++start;
    }
}

template <class InputMgr>
void HistApprox<InputMgr>::addEdge(const int u, const int v, const int l) {
    createAlgIfNecessary(l);
    for (auto it = algs_.begin(); it != algs_.end() && *it->l_ <= l; ++it)
        *it->sieve_ptr_->addEdge(u, v);
}

template <class InputMgr>
double HistApprox<InputMgr>::update() {
    for (auto it = algs_.begin(); it != algs_.end(); ++it) {
        *it->sieve_ptr_->update();
        *it->val_ = *it->sieve_ptr_->getResult().second;
    }
    return algs_.begin()->val_;
}

template <class InputMgr>
void HistApprox<InputMgr>::clear() {
    if (algs_.begin()->l_ == 1) {
        delete algs_.front();
        algs_.pop_front();
    }
    for (auto it = algs_.begin(); it != algs_.end(); ++it) {
        *it->sieve_ptr_->clear();  // only clear temporal results
        *it->l_--;
    }
}

#endif /* __HIST_APPROX_H__ */
