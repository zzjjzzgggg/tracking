#include "../stdafx.h"
#include "dim.hpp"

#include <gflags/gflags.h>

DEFINE_string(graph, "", "input graph");
DEFINE_string(idmap, "", "input nd_id map");

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

    DIM dim;
    dim.init();        // Call at the beginning
    dim.set_beta(32);  // Set beta=32

    std::unordered_set<int> nodes;

    ioutils::TSVParser ss(FLAGS_graph, ' ');
    while (ss.next()) {
        int u = ss.get<int>(0), v = ss.get<int>(1);
        double p = ss.get<double>(2);

        if (nodes.find(u) == nodes.end()) {
            dim.insert(u);
            nodes.insert(u);
        }

        if (nodes.find(v) == nodes.end()) {
            dim.insert(v);
            nodes.insert(v);
        }

        dim.insert(u, v, p);
    }

    auto seeds = dim.infmax(10);
    printf("influence: %.4f\n", dim.infest(seeds));

    auto id_nd = ioutils::loadMap<int, int>(FLAGS_idmap, 1, 0);
    for (int& v : seeds) v = id_nd[v];
    ioutils::saveVec(seeds, "dim_seeds.dat");

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
