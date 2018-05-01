/**
 * Copyright (C) by J.Z. (04/05/2018 19:28)
 * Distributed under terms of the MIT license.
 */
#ifndef DGRAPH
#include "dyn_bgraph_mgr.h"
#else
#include "dyn_dgraph_mgr.h"
#endif

#include "greedy_alg.h"

#include <gflags/gflags.h>

DEFINE_int32(budget, 10, "budget");
DEFINE_int32(end_tm, 100, "end time");
DEFINE_int32(batch_sz, 10, "batch size");
DEFINE_string(graph, "", "input bipartite graph (user, venue, time)");

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

    printf("\ttime\tvalue\toracl-calls\tnaive-oc\n");

    int t = 0, num = 0, oracle_calls = 0, naive_oracle_calls = 0;
    ioutils::TSVParser ss(FLAGS_graph);
    while (ss.next()) {
        int u = ss.get<int>(0), v = ss.get<int>(1);
        input_mgr.addEdge(u, v);
        ++num;
        if (num == FLAGS_batch_sz) {
            input_mgr.getAffectedNodes();
            double val = greedy.run();
            oracle_calls += greedy.getOracleCalls();
            naive_oracle_calls += input_mgr.getNumNodes() * FLAGS_budget;

            input_mgr.clear();

            rst.emplace_back(++t, val, oracle_calls, naive_oracle_calls);
            num = 0;

            printf("\t%d\t%.0f\t%6d\t%6d\r", t, val, oracle_calls,
                   naive_oracle_calls);
            fflush(stdout);
        }
        if (t == FLAGS_end_tm) break;
    }
    printf("\n");

    // save results
    std::string ofnm = strutils::insertMiddle(
        FLAGS_graph, "greedy_adn_k{}"_format(FLAGS_budget), "dat");
    std::string ano =
        "#graph: {}\nbudget: {}\n#batch size: {}\n#end time: {}\n"_format(
            FLAGS_graph, FLAGS_budget, FLAGS_batch_sz, FLAGS_end_tm);
    ioutils::saveTupleVec(rst, ofnm, true, "{}\t{:.1f}\t{}\t{}\n", ano);

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
