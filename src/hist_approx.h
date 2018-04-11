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
        int l_, i_;
        double val_;  // current solution value of this algorithm instance

        SieveADN<InputMgr>* sieve_ptr_;

    public:
        Alg(const int l, const int budget, const double eps)
            : l_(l), i_(-100), val_(0) {
            sieve_ptr_ = new SieveADN<InputMgr>(budget, eps);
        }
        virtual ~Alg() { delete sieve_ptr_; }

        /**
         * Copy constructor.
         */
        Alg(const Alg& o) : l_(o.l_), i_(o.i_), val_(o.val_) {
            sieve_ptr_ = new SieveADN<InputMgr>(*o.sieve_ptr_);
        }

        void clear() { sieve_ptr_->clear(); }

        inline void updateVal() {
            sieve_ptr_->update();
            auto rst = sieve_ptr_->getResult();
            i_ = rst.first;
            val_ = rst.second;
        }

        inline void addEdges(const std::vector<IntPr>& edges) {
            sieve_ptr_->addEdges(edges);
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
    void bufEdges(const std::vector<IntPr>& edges, const int l) {
        auto& target = edge_buf_[(cur_ + l - 1) % L_];
        target.insert(target.end(), edges.begin(), edges.end());
    };

    /**
     * Feed a batch of edges with lifetime l to instances with l' <= l.
     * Need to scan alg. instances in in the list from head to tail.
     */
    void addEdges(const std::vector<IntPr>& edges, const int l) {
        auto it = algs_.begin();
        while (it != algs_.end() && (*it)->l_ <= l) {
            (*it)->addEdges(edges);
            (*it)->updateVal();
            ++it;
        }
        // check whether or not need to create a new Alg instance
        if (it == algs_.end()) {  // reach tail?
            auto tail = algs_.rbegin();
            // create a new Alg if the list is empty or its tail has smaller l
            if (tail == algs_.rend() || (*tail)->l_ < l) {
                Alg* alg = new Alg(l, budget_, eps_);
                alg->addEdges(edges);
                alg->updateVal();
                algs_.insert(algs_.end(), alg);
            }
        } else {
            auto ancestor = it;
            // create an Alg based on its ancestor
            if (ancestor == algs_.begin() || (*--it)->l_ < l) {
                Alg* alg = new Alg(*(*ancestor));
                alg->l_ = l;
                // process new edges
                alg->addEdges(edges);
                // process history edges with lifetime in range [l, l^*)
                for (int ll = l; ll < (*ancestor)->l_; ll++) {
                    const auto& history = edge_buf_[(cur_ + ll - 1) % L_];
                    if (!history.empty()) alg->addEdges(history);
                }
                alg->updateVal();
                algs_.insert(ancestor, alg);
            }
        }
        rmRedundancy();
    }

    std::pair<int, double> getResult() const {
        return std::make_pair(algs_.front()->i_, algs_.front()->val_);
    }

    std::vector<int> getSolution(const int i) const {
        return algs_.front()->sieve_ptr_->getSolution(i);
    }

    std::string getInfo() const {
        return fmt::format("{:5d}", algs_.size());
    }
    /**
     * Prepare for next time step. Deleta the head instance if its l=1, and
     * conduct light clean for other instances.
     */
    void clear();

}; /* HistApprox */

// implementations
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
            while (++it != last) delete *it;
            algs_.erase(++start, last);
            start = last;
        } else
            ++start;
    }
}

template <class InputMgr>
void HistApprox<InputMgr>::clear() {
    if (algs_.front()->l_ == 1) {
        delete algs_.front();
        algs_.pop_front();
    }
    for (auto it = algs_.begin(); it != algs_.end(); ++it) {
        (*it)->clear();  // only clear temporal results
        (*it)->l_--;
    }

    // clear outdated edges
    edge_buf_[cur_].clear();
    cur_ = (cur_ + 1) % L_;
}

#endif /* __HIST_APPROX_H__ */
