/**
 * Copyright (C) by J.Z. (04/05/2018 19:28)
 * Distributed under terms of the MIT license.
 */
#include "dyn_bgraph_mgr.h"
#include "sieve_adn.h"

#include <gflags/gflags.h>

DEFINE_int32(budget, 10, "budget");
DEFINE_double(eps, 0.1, "epsilon");
DEFINE_string(graph, "", "input bipartite graph (user, venue, time)");

int main(int argc, char *argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

    DynBGraphMgr bgraph_mgr;
    SieveADN sieveADN{bgraph_mgr, FLAGS_budget, FLAGS_eps};

    std::vector<std::pair<int, int>> edges;

    int n = 0;
    ioutils::TSVParser ss(FLAGS_graph);
    while (ss.next()) {
        int u = ss.get<int>(0), v = ss.get<int>(1);
        edges.emplace_back(u, v);
        if (edges.size() % 10 == 0) {
            sieveADN.processBatch(edges);
            printf("rwd: %.2f\n", sieveADN.getResult());
            edges.clear();
            n++;
        }

        if (n == 10) break;
    }

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
