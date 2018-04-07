/**
 * Copyright (C) by J.Z. (04/05/2018 19:28)
 * Distributed under terms of the MIT license.
 */
#include "dyn_bgraph_mgr.h"
#include "sieve_adn.h"

#include <gflags/gflags.h>

DEFINE_string(graph, "", "input bipartite graph (user, venue, ...)");
DEFINE_int32(budget, 100, "budget");
DEFINE_int32(end_tm, 100, "end time");
DEFINE_int32(batch_sz, 100, "batch size");
DEFINE_double(eps, 0.5, "epsilon");

int main(int argc, char *argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

    SieveADN<DynBGraphMgr> sieveADN{FLAGS_budget, FLAGS_eps};

    std::vector<std::pair<int, int>> edges;
    std::vector<std::pair<int, double>> tm_rwd;

    int t = 0;
    ioutils::TSVParser ss(FLAGS_graph);
    while (ss.next()) {
        int u = ss.get<int>(0), v = ss.get<int>(1);
        edges.emplace_back(u, v);
        if (edges.size() == FLAGS_batch_sz) {
            sieveADN.addEdges(edges);
            sieveADN.update();

            auto rst = sieveADN.getResult();
            printf("\t%d\t\t%.0f\r", ++t, rst.second);
            fflush(stdout);
            rst.first = t;
            tm_rwd.push_back(std::move(rst));

            edges.clear();
        }
        if (t == FLAGS_end_tm) break;
    }
    printf("\n");

    // save results
    std::string ofnm = strutils::insertMiddle(
        FLAGS_graph, "SieveAdn_E{:g}"_format(FLAGS_eps), "dat");
    std::string ano = fmt::format(
        "#graph: {}\n#budget: {}\n#batch size: {}\n#end time: {}\n#epsilon: "
        "{:.2f}\n",
        FLAGS_graph, FLAGS_budget, FLAGS_batch_sz, FLAGS_end_tm, FLAGS_eps);
    ioutils::savePrVec(tm_rwd, ofnm, true, "{:d}\t{:.1f}\n", ano);

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
