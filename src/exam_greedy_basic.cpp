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
#include "eval_stream.h"

#include <gflags/gflags.h>

DEFINE_string(graph, "", "input graph");
DEFINE_int32(budget, 10, "budget");
DEFINE_int32(end_tm, 1000, "end time");
DEFINE_int32(L, 10000, "maximum lifetime");
DEFINE_bool(save, true, "save results or not");

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

#ifndef DGRAPH
    EvalStream<DynBGraphMgr> eval(FLAGS_L);
#else
    EvalStream<DynDGraphMgr> eval(FLAGS_L);
#endif

    std::vector<std::tuple<int, double, int, int, int>> rst;

    printf("\t%-12s%-12s%-12s%-12s\n", "time", "value", "#calls", "#nv_calls");

    int t = 0, ocalls = 0, nv_ocalls = 0;
    double val = 0;
    ioutils::TSVParser ss(FLAGS_graph);
    while (ss.next()) {
        int u = ss.get<int>(0), v = ss.get<int>(1), l = ss.get<int>(2);
        eval.addEdge(u, v, l);
        ++t;

        auto input_mgr = eval.getInputMgr();
        GreedyAlg greedy(&input_mgr, FLAGS_budget);
        val = greedy.run();
        // greedy.saveSolution("greedy_sol_v2.txt");
        ocalls += greedy.getOracleCalls();

        int graph_sz = input_mgr.getNumNodes();
        nv_ocalls += graph_sz * FLAGS_budget;

        rst.emplace_back(t, val, ocalls, nv_ocalls, graph_sz);

        eval.next();

        printf("\t%-12d%-12.0f%-12d%-12d\r", t, val, ocalls, nv_ocalls);
        fflush(stdout);

        if (t == FLAGS_end_tm) break;
    }
    printf("\n");

    // save results
    if (FLAGS_save) {
        std::string ofnm = strutils::insertMiddle(
            FLAGS_graph,
            "greedy_basic_K{}T{}"_format(FLAGS_budget,
                                         strutils::prettyNumber(FLAGS_end_tm)),
            "dat");
        ioutils::saveTupleVec(rst, ofnm, "{}\t{:.2f}\t{}\t{}\t{}\n");
    }

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
