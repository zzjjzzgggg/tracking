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
    /**
     * SieveADN instance.
     * Only process edges with lifetime no less than l
     */
    class Alg {
    public:
        int l_;
        double val_ = 0;  // current solution value of this algorithm instance

        SieveADN<InputMgr>* sieve_ptr_;

    public:
        Alg(const int l, const int budget, const double eps) : l_(l) {
            sieve_ptr_ = new SieveADN<InputMgr>(budget, eps);
        }
        virtual ~Alg() { delete sieve_ptr_; }

        // Copy constructor.
        Alg(const Alg& o) : l_(o.l_), val_(o.val_) {
            sieve_ptr_ = new SieveADN<InputMgr>(*o.sieve_ptr_);
        }

        inline void feedEdges(const IntPrV& edges) {
            sieve_ptr_->feedEdges(edges);
        }

        inline void update() {
            sieve_ptr_->update();
            val_ = sieve_ptr_->getResult().second;
        }

        inline void clear() { sieve_ptr_->clear(); }

        inline int getOracleCalls() { return sieve_ptr_->getOracleCalls(); }

    }; /* Alg */

private:
    int L_, budget_;
    double eps_;
    int cur_ = 0, del_calls_ = 0;

    std::list<Alg*> algs_;  // a list of Alg instances
    std::vector<IntPrV> edge_buf_;

private:
    /**
     * Remove epsion-reduction alg instances from the list according instances'
     * current output.
     */
    void rmRedundancy();

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
    void bufEdges(const int l, const IntPrV& edges) {
        auto& target = edge_buf_[(cur_ + l - 1) % L_];
        target.insert(target.end(), edges.begin(), edges.end());
    };

    /**
     * Feed a batch of edges with lifetime l to instances with l' <= l.
     * Need to scan alg. instances in in the list from head to tail.
     */
    void feedAndUpdate(const int l, const IntPrV& edges);

    double getResult() const { return algs_.front()->val_; }

    int statOracleCalls();
    int getNumAlgs() const { return algs_.size(); }

    /**
     * Prepare for next time step. Deleta the head instance if its l=1, and
     * conduct light clean for other instances.
     */
    void next();

}; /* HistApprox */

// implementations
template <class InputMgr>
void HistApprox<InputMgr>::feedAndUpdate(const int l, const IntPrV& edges) {
    auto it = algs_.begin();
    while (it != algs_.end() && (*it)->l_ <= l) {
        (*it)->feedEdges(edges);
        (*it)->update();
        ++it;
    }

    auto succ = it;

    // check whether or not need to create a new Alg instance
    if (succ == algs_.end()) {  // reach tail?
        auto tail = algs_.rbegin();
        // create a new Alg if the list is empty or its tail has smaller l
        if (tail == algs_.rend() || (*tail)->l_ < l) {
            Alg* alg = new Alg(l, budget_, eps_);
            alg->feedEdges(edges);
            alg->update();
            algs_.insert(succ, alg);
        }
    } else {
        bool flag =
            (succ == algs_.begin())
                ? true
                : (*--it)->l_ < l && (*succ)->val_ < (*it)->val_ * (1 - eps_);
        // create an Alg based on its succ
        if (flag) {
            Alg* alg = new Alg(*(*succ));
            alg->l_ = l;
            alg->clear();
            alg->feedEdges(edges);
            for (int ll = l; ll < (*succ)->l_; ll++) {
                const auto& history = edge_buf_[(cur_ + ll - 1) % L_];
                if (!history.empty()) alg->feedEdges(history);
            }
            alg->update();
            algs_.insert(succ, alg);
        }
    }
    // rmRedundancy();
}

template <class InputMgr>
void HistApprox<InputMgr>::rmRedundancy() {
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
            while (++it != last) {
                del_calls_ += (*it)->getOracleCalls();
                delete *it;
            }
            algs_.erase(++start, last);
            start = last;
        } else
            ++start;
    }
}

template <class InputMgr>
int HistApprox<InputMgr>::statOracleCalls() {
    int oracle_calls = del_calls_;
    for (auto it = algs_.begin(); it != algs_.end(); ++it)
        oracle_calls += (*it)->getOracleCalls();
    del_calls_ = 0;
    return oracle_calls;
}

template <class InputMgr>
void HistApprox<InputMgr>::next() {
    del_calls_ = 0;

    // If head SieveADN instance expires
    if (algs_.front()->l_ == 1) {
        delete algs_.front();
        algs_.pop_front();
    }

    // clear remaining SieveADN instances
    for (auto it = algs_.begin(); it != algs_.end(); ++it) {
        (*it)->clear();
        (*it)->l_--;
    }

    // clear outdated edges
    edge_buf_[cur_].clear();
    cur_ = (cur_ + 1) % L_;
}

#endif /* __HIST_APPROX_H__ */
