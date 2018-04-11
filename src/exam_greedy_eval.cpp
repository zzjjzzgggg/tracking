/**
 * Copyright (C) by J.Z. (04/05/2018 19:28)
 * Distributed under terms of the MIT license.
 */
#include "dyn_bgraph_mgr.h"
#include "basic_reduction.h"
#include "greedy_alg.h"
#include "eval_stream.h"

#include <gflags/gflags.h>

DEFINE_string(graph, "", "input bipartite graph (user, venue, ...)");
DEFINE_int32(budget, 50, "budget");
DEFINE_int32(end_tm, 100, "end time");
DEFINE_int32(batch_sz, 10, "batch size");
DEFINE_int32(L, 10, "maximum lifetime");

int main(int argc, char *argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

    EvalStream<DynBGraphMgr> eval(FLAGS_L);

    std::vector<std::pair<int, double>> tm_val;

    int t = 0, num = 0;
    ioutils::TSVParser ss(FLAGS_graph);
    while (ss.next()) {
        int u = ss.get<int>(0), v = ss.get<int>(1), l = ss.get<int>(2);
        eval.addEdge(u, v, l);
        ++num;
        if (num == FLAGS_batch_sz) {
            auto& input_mgr = eval.getInputMgr();
            // input_mgr.getGraphStat();

            GreedyAlg greedy(&input_mgr, FLAGS_budget);
            double val = greedy.run();

            eval.clear();
            num = 0;

            printf("\t%d\t\t%.0f\r", ++t, val);
            fflush(stdout);
            tm_val.emplace_back(t, val);
        }
        if (t == FLAGS_end_tm) break;
    }
    printf("\n");

    // save results
    std::string ofnm = strutils::insertMiddle(
        FLAGS_graph, "greedy_basic_k{}"_format(FLAGS_budget), "dat");
    std::string ano =
        "#graph: {}\nbudget: {}\n#batch size: {}\n#end time: {}\n"_format(
            FLAGS_graph, FLAGS_budget, FLAGS_batch_sz, FLAGS_end_tm);
    ioutils::savePrVec(tm_val, ofnm, true, "{:d}\t{:.1f}\n", ano);

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
