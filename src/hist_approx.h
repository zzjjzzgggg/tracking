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
     * SieveADN instance. Only process edges with lifetime no less than l.
     */
    class Alg {
    private:
        SieveADN<InputMgr>* sieve_ptr_;

    public:
        int l_;
        double val_ = 0;

    public:
        Alg(const int l, const int budget, const double eps) : l_(l) {
            sieve_ptr_ = new SieveADN<InputMgr>(budget, eps);
        }
        virtual ~Alg() { delete sieve_ptr_; }

        // Copy constructor.
        Alg(const Alg& o) : l_(o.l_), val_(o.val_) {
            sieve_ptr_ = new SieveADN<InputMgr>(*o.sieve_ptr_);
        }

        inline void feedEdge(const int u, const int v) {
            sieve_ptr_->feedEdge(u, v);
        }

        inline void feedEdges(const IntPrV& edges) {
            if (edges.empty()) return;
            sieve_ptr_->feedEdges(edges);
        }

        inline void update() {
            sieve_ptr_->update();
            val_ = sieve_ptr_->getResult().second;
        }

        inline void clear() { sieve_ptr_->clear(); }

        inline int getOracleCalls() { return sieve_ptr_->getOracleCalls(); }

        std::vector<int> getSolution() const {
            int i = sieve_ptr_->getResult().first;
            return sieve_ptr_->getSolution(i);
        }

        void debug() const {
            printf("\n=======debug start========\n");

            sieve_ptr_->debug();

            auto nodes = ioutils::loadVec<int>("sol.txt");
            auto& input_mgr = sieve_ptr_->getInputMgr();
            double val = input_mgr.getReward(nodes);
            printf("reward by front SieveADN: %.2f\n", val);

            InputMgr new_input_mgr;
            ioutils::TSVParser ss("edges_v2.dat");
            while (ss.next()) {
                int u = ss.get<int>(0), v = ss.get<int>(1);
                new_input_mgr.addEdge(u, v);
            }
            val = new_input_mgr.getReward(nodes);
            printf("reward by new input manager: %.2f\n", val);

            printf("\n=======debug end========\n");
        }

    }; /* Alg */

private:
    int L_, budget_;
    double eps_;
    int cur_ = 0, del_calls_ = 0;

    std::list<Alg*> algs_;  // a list of Alg instances
    std::vector<IntPrV> edge_buf_;

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
    }

    void bufEdge(const int u, const int v, const int l) {
        edge_buf_[(cur_ + l - 1) % L_].emplace_back(u, v);
    }

    /**
     * Feed an edge with lifetime l to instances with l' <= l.
     * Need to scan alg. instances in in the list from head to tail.
     */
    void process(const int u, const int v, const int l);

    /**
     * Remove epsion-reduction alg instances from the list according instances'
     * current output.
     */
    void reduce();

    double getResult() const { return algs_.front()->val_; }
    void saveSolution(const std::string& filename) const {
        ioutils::saveVec(algs_.front()->getSolution(), filename);
    }

    int statOracleCalls();
    int getNumAlgs() const { return algs_.size(); }

    /**
     * Prepare for next time step. Deleta the head instance if its l=1, and
     * conduct light clean for other instances.
     */
    void next();

    void debug() const { algs_.front()->debug(); }

}; /* HistApprox */

// implementations
template <class InputMgr>
void HistApprox<InputMgr>::process(const int u, const int v, const int l) {
    // if list is empty or its tail has smaller l, then create new alg
    if (algs_.empty() || algs_.back()->l_ < l)
        algs_.push_back(new Alg(l, budget_, eps_));

    auto job = [u, v](Alg* alg) {
        alg->feedEdge(u, v);
        alg->update();
    };
    std::vector<std::future<void>> futures;
    // update each alg with l_<=l
    auto it = algs_.begin();
    for (; it != algs_.end() && (*it)->l_ <= l; ++it)
        futures.push_back(std::async(std::launch::async, job, *it));
    for (auto& future : futures) future.get();
    if (it == algs_.end()) return;

    auto succ = it, prev = --it;

    // if last updated alg.l_ = l, then return
    if (succ != algs_.begin() && (*prev)->l_ == l) return;

    // create new alg before succ
    if ((succ == algs_.begin()) || (*succ)->val_ < (*prev)->val_ * (1 - eps_)) {
        // create alg_l based on its successor
        Alg* alg = new Alg(*(*succ));
        alg->l_ = l;
        alg->clear();
        alg->feedEdge(u, v);
        for (int ll = l; ll < (*succ)->l_; ll++)
            alg->feedEdges(edge_buf_[(cur_ + ll - 1) % L_]);
        alg->update();
        algs_.insert(succ, alg);
    }
}

template <class InputMgr>
void HistApprox<InputMgr>::reduce() {
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
    int i = 0;
    for (auto it = algs_.begin(); it != algs_.end(); ++it) {
        (*it)->clear();
        (*it)->l_--;
    }
    // clear outdated edges
    edge_buf_[cur_].clear();
    cur_ = (cur_ + 1) % L_;
}

#endif /* __HIST_APPROX_H__ */
