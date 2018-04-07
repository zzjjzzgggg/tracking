/**
 * Copyright (C) by J.Z. (04/05/2018 19:28)
 * Distributed under terms of the MIT license.
 */
#include "dyn_bgraph_mgr.h"
#include "greedy_alg.h"

#include <gflags/gflags.h>

DEFINE_int32(budget, 100, "budget");
DEFINE_int32(end_tm, 200, "end time");
DEFINE_int32(batch_sz, 100, "batch size");
DEFINE_string(graph, "", "input bipartite graph (user, venue, time)");

int main(int argc, char *argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

    DynBGraphMgr input_mgr;
    GreedyAlg greedy{&input_mgr, FLAGS_budget};

    std::vector<std::pair<int, int>> edges;
    std::vector<std::pair<int, double>> tm_rwd;

    int t = 0;
    ioutils::TSVParser ss(FLAGS_graph);
    while (ss.next()) {
        int u = ss.get<int>(0), v = ss.get<int>(1);
        edges.emplace_back(u, v);
        if (edges.size() == FLAGS_batch_sz) {
            input_mgr.addEdges(edges);
            double rwd = greedy.run();

            printf("\t%d\t\t%.0f\r", ++t, rwd);
            fflush(stdout);
            tm_rwd.emplace_back(t, rwd);
            edges.clear();
        }
        if (t == FLAGS_end_tm) break;
    }
    printf("\n");

    // save results
    std::string ofnm = strutils::insertMiddle(FLAGS_graph, "greedy_adn", "dat");
    std::string ano =
        "#graph: {}\nbudget: {}\n#batch size: {}\n#end time: {}\n"_format(
            FLAGS_graph, FLAGS_budget, FLAGS_batch_sz, FLAGS_end_tm);
    ioutils::savePrVec(tm_rwd, ofnm, true, "{:d}\t{:.1f}\n", ano);

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
