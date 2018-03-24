/*
 * Copyright (C) 2017 jzzhao <jzzhao@zhlap>
 * Distributed under terms of the MIT license.
 */

#include "config.h"

DEFINE_string(graph, "", "input graph filename");
DEFINE_string(walks_dir, "", "walks subdirectory");
DEFINE_int32(num_nodes, 0, "number of nodes");
DEFINE_int32(walks_per_node, 10, "number of walks per node");
DEFINE_int32(hops, 10, "walk length");
DEFINE_int32(cores, std::thread::hardware_concurrency(), "number of cores");
DEFINE_int32(depth, 0, "refinement depth");

int getByte(const int idx, const vector<int>& vec) {
    return vec[idx / 4] >> (idx % 4 * 8) & 0xff;
}

void setByte(const int idx, const int val, vector<int>& vec) {
    vec[idx / 4] |= 0xff << (idx % 4 * 8);
    vec[idx / 4] ^= (~val & 0xff) << (idx % 4 * 8);
}

int get4BytesHop(const int hop_val) {
    int hop = hop_val & 0xff;
    hop = hop << 8 | hop;
    hop = hop << 16 | hop;
    return hop;
}

Config::Config() {
    n_nodes = FLAGS_num_nodes;
    walks = FLAGS_walks_per_node;
    hops = FLAGS_hops - FLAGS_depth;
    n_cpus = FLAGS_cores;
    depth = FLAGS_depth;
    graph_fnm = FLAGS_graph;
    dir = strutils::getBasePath(graph_fnm);
    walks_dir = osutils::join(dir, FLAGS_walks_dir);

    weight_strategy = WeightStrategy::Unit5;

    total_walks = n_nodes * walks;
    n_bkts = GET_BUCKET(n_nodes) + 1;
    n_bkts_a_cpu = std::min(n_bkts / n_cpus + 1, n_bkts);
    n_nodes_a_cpu = std::min(FIRST_NODE_IN_BUCKET(n_bkts_a_cpu), n_nodes);
}

void Config::setWalksDir(const string& walks_dir) {
    this->walks_dir = osutils::join(dir, walks_dir);
}

void Config::setDepth(const int depth) {
    this->depth = depth;
    hops = FLAGS_hops - depth;
}

void Config::info() {
    printf("%s, %s\n|V|: %d, R: %d, T: %d, |CPU|: %d, D: %d, WT: %d\n",
           graph_fnm.c_str(), walks_dir.c_str(), n_nodes, walks, hops, n_cpus,
           depth, (int)weight_strategy);
}

bool ChosenSet::all_connect = false;

unordered_map<int, double> ChosenSet::absb_edges;
