/**
 * Copyright (C) by J.Z. (04/05/2018 19:28)
 * Distributed under terms of the MIT license.
 */
#ifndef DGRAPH
#include "dyn_bgraph_mgr.h"
#else
#include "dyn_dgraph_mgr_v2.h"
#endif

#include "greedy_alg.h"

#include <gflags/gflags.h>

DEFINE_string(graph, "", "input bipartite graph (user, venue, time)");
DEFINE_int32(budget, 10, "budget");
DEFINE_int32(end_tm, 100, "end time");
DEFINE_int32(batch, 1, "batch size");
DEFINE_bool(save, true, "save results or not");

int main(int argc, char *argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

#ifndef DGRAPH
    DynBGraphMgr input_mgr;
#else
    DynDGraphMgr input_mgr;
#endif
    GreedyAlg greedy{&input_mgr, FLAGS_budget};

    std::vector<std::tuple<int, double, int, int>> rst;

    printf("\t%-12s%-12s%-12s%-12s\n", "time", "value", "#calls", "#nv_calls");

    int t = 0, num = 0, ocalls = 0, naive_ocalls = 0;
    ioutils::TSVParser ss(FLAGS_graph);
    while (ss.next()) {
        int u = ss.get<int>(0), v = ss.get<int>(1);
        input_mgr.addEdge(u, v);
        ++num;
        if (num == FLAGS_batch) {
            input_mgr.getAffectedNodes();
            double val = greedy.run();
            ocalls += greedy.getOracleCalls();
            naive_ocalls += input_mgr.getNumNodes() * FLAGS_budget;

            input_mgr.clear();

            rst.emplace_back(++t, val, ocalls, naive_ocalls);
            num = 0;

            printf("\t%-12d%-12.0f%-12d%-12d\r", t, val, ocalls, naive_ocalls);
            fflush(stdout);
        }
        if (t == FLAGS_end_tm) break;
    }
    printf("\n");

    if (FLAGS_save) {
        // save results
        std::string ofnm = strutils::insertMiddle(
            FLAGS_graph, "greedy_adn_k{}"_format(FLAGS_budget), "dat");
        std::string ano =
            "#graph: {}\nbudget: {}\n#batch size: {}\n#end time: {}\n"_format(
                FLAGS_graph, FLAGS_budget, FLAGS_batch, FLAGS_end_tm);
        ioutils::saveTupleVec(rst, ofnm, true, "{}\t{:.1f}\t{}\t{}\n", ano);
    }

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
