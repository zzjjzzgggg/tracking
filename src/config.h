#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "stdafx.h"
#include "walk_encoder.h"
#include <gflags/gflags.h>

DECLARE_string(graph);
DECLARE_string(walks_dir);
DECLARE_int32(n_nodes);
DECLARE_int32(walks);
DECLARE_int32(hops);
DECLARE_int32(cores);
DECLARE_int32(depth);

int getByte(const int idx, const vector<int>& vec);
void setByte(const int idx, const int val, vector<int>& vec);
int get4BytesHop(const int hop_val);

enum class WeightStrategy {
    Unit = 1,     // an absorbing edge has weight 1
    Unit2 = 2,    // an absorbing edge has weight 2
    Unit5 = 5,    // an absorbing edge has weight 5
    Unit10 = 10,  // an absorbing edge has weight 10
    Degree,       // equal to the degree of the node connected from
    DegreeLog     // log of the degree + 1, i.e., 1 + log(degree)
};

class AbsbManager {
public:
    AbsbManager() {}

    static double getAbsbWeight(const int out_deg,
                                const WeightStrategy strategy) {
        switch (strategy) {
            case WeightStrategy::Unit:
                return 1;
            case WeightStrategy::Unit2:
                return 2;
            case WeightStrategy::Unit5:
                return 5;
            case WeightStrategy::Unit10:
                return 10;
            case WeightStrategy::Degree:
                return out_deg;
            case WeightStrategy::DegreeLog:
                return std::log(out_deg) + 1;
            default:
                return 1;
        }
    }
};

struct Config {
    int n_nodes;
    int walks;
    int hops;
    int n_cpus;
    int depth;
    string graph_fnm;
    string walks_dir;

    string dir;
    WeightStrategy weight_strategy;
    int total_walks;
    int n_bkts;
    int n_bkts_a_cpu;
    int n_nodes_a_cpu;
    int absb_node{-1};

    Config();

    /**
     * return the position index of a walk in a vector
     */
    inline int walkIdx(const long walk) const {
        return Encoder::getSource(walk) * walks + Encoder::getId(walk);
    }

    double getAbsbWt(const int deg) const {
        return AbsbManager::getAbsbWeight(deg, weight_strategy);
    }

    void setWalksDir(const string& new_walks_dir);
    void setDepth(const int depth);

    void info();
};

class ChosenSet {
private:
    // whether or not connect every node to the absorbing node
    static bool all_connect;
    // a node who has connection with target --> edge weight
    static std::unordered_map<int, double> absb_edges;

public:
    static void setAllConnection() {
        all_connect = true;
        printf("All connection: true\n");
    }

    static bool isConnected(const int node) {
        return all_connect || absb_edges.find(node) != absb_edges.end();
    }

    static void add(const int src, double weight = 1) {
        absb_edges[src] = weight;
    }

    static void remove(const int src) { absb_edges.erase(src); }

    static void load(const string& nodes_fnm) {}
};

class NodeContext {
private:
    randutils::default_rng rng_;

public:
    int node_;
    double sum_weights_;
    std::vector<int> nbrs_uni_, nbrs_non_uni_;
    std::vector<double> weights_;

public:
    NodeContext(const int node = -1) : node_(node), sum_weights_(0) {}

    void addNbr(const int nbr, const double weight = -1) {
        if (weight < 0)
            nbrs_uni_.push_back(nbr);
        else {
            nbrs_non_uni_.push_back(nbr);
            weights_.push_back(weight);
            sum_weights_ += weight;
        }
    }

    int size() const { return nbrs_uni_.size() + nbrs_non_uni_.size(); }

    int forward() {
        int n_uni_nbrs = nbrs_uni_.size();
        if (rng_.uniform() * (n_uni_nbrs + sum_weights_) < n_uni_nbrs) {
            return nbrs_uni_[rng_.uniform(0, n_uni_nbrs - 1)];
        } else {
            // assert_msg(sum_weights_ > 0, "node: %d", node_);
            // assert_msg(!weights_.empty(), "node: %d", node_);
            return nbrs_non_uni_[randutils::sample(weights_, sum_weights_,
                                                   rng_)];
        }
    }
};

#endif /* __CONFIG_H__ */
