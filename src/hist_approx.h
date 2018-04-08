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
        Alg(const Alg& o) : l_(o.l_), val_(o.val_) {
            sieve_ptr_ = new SieveADN<InputMgr>(*o.sieve_ptr_);
        }

    }; /* Alg */

private:
    int L_, budget_;
    double eps_;

    std::list<Alg*> algs_;  // a list of alg instances

    int cur_ = 0;
    std::vector<std::vector<IntPr>> edge_buf_;

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
    HistApprox(const int L, const int budget, const double eps)
        : L_(L), budget_(budget), eps_(eps) {
        edge_buf_.resize(L);
    }
    virtual ~HistApprox() {
        for (auto it = algs_.begin(); it != algs_.end(); ++it) delete *it;
    }

    /**
     * Store edges with lifetime l into buffer.
     */
    void bufEdges(const std::vector<IntPr>& edges, const int l);

    /**
     * Feed a batch of edges with lifetime l to instances with l' <= l.
     * Need to scan alg. instances in in the list from head to tail.
     */
    void addEdges(const std::vector<IntPr>& edges, const int l);

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
    // linear search
    auto it = algs_.begin();
    while (it != algs_.end() && (*it)->l_ < l) ++it;

    // then, create an alg instance before it
    if (it == algs_.end())  // l has no successor, then create a new ins
        algs_.insert(it, new Alg(l, budget_, eps_));
    else if ((*it)->l_ > l) {  // create a copy of its successor
        Alg* alg = new Alg(*(*it));
        // Alg* alg = new Alg(l, budget_, eps_);
        // process edges with lifetime in range [l, (*it)->l_)
        // for (int ll = l; ll < (*it)->l_; ll++) {
        //     const auto& history = edge_buf_[(cur_ + ll - 1) % L_];
        //     if (!history.empty()) alg->sieve_ptr_->addEdges(history);
        // }
        algs_.insert(it, alg);
    }
}

template <class InputMgr>
void HistApprox<InputMgr>::removeRedundancy() {
    auto start = algs_.begin();
    while (start != algs_.end()) {
        double bound = (*start)->val_ * (1 - eps_);
        auto last = start, it = start;
        while (true) {
            ++it;
            if (it == algs_.end() || (*it)->val_ < bound) break;
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
void HistApprox<InputMgr>::bufEdges(const std::vector<IntPr>& edges,
                                    const int l) {
    auto& target = edge_buf_[(cur_ + l - 1) % L_];
    target.insert(target.end(), edges.begin(), edges.end());
}

template <class InputMgr>
void HistApprox<InputMgr>::addEdges(const std::vector<IntPr>& edges,
                                    const int l) {
    createAlgIfNecessary(l);
    for (auto it = algs_.begin(); it != algs_.end() && (*it)->l_ <= l; ++it) {
        (*it)->sieve_ptr_->addEdges(edges);
        (*it)->sieve_ptr_->update();
        (*it)->val_ = (*it)->sieve_ptr_->getResult().second;
    }
    removeRedundancy();
}

template <class InputMgr>
double HistApprox<InputMgr>::update() {
    // for (auto it = algs_.begin(); it != algs_.end(); ++it) {
    //     (*it)->sieve_ptr_->update();
    //     (*it)->val_ = (*it)->sieve_ptr_->getResult().second;
    // }
    return algs_.front()->val_;
}

template <class InputMgr>
void HistApprox<InputMgr>::clear() {
    if (algs_.front()->l_ == 1) {
        delete algs_.front();
        algs_.pop_front();
    }
    for (auto it = algs_.begin(); it != algs_.end(); ++it) {
        (*it)->sieve_ptr_->clear();  // only clear temporal results
        (*it)->l_--;
    }
    // clear outdated edges
    edge_buf_[cur_].clear();
    cur_ = (cur_ + 1) % L_;
}

#endif /* __HIST_APPROX_H__ */
