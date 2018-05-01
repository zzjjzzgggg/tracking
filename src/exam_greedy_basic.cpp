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
#include "eval_stream.h"

#include <gflags/gflags.h>

DEFINE_string(graph, "", "input graph");
DEFINE_int32(budget, 10, "budget");
DEFINE_int32(end_tm, 100, "end time");
DEFINE_int32(batch_sz, 10, "batch size");
DEFINE_int32(L, 10, "maximum lifetime");
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

    int t = 0, num = 0, oracle_calls = 0, naive_oracle_calls = 0;
    ioutils::TSVParser ss(FLAGS_graph);
    printf("\ttime\tvalue\toracl-calls\n");
    while (ss.next()) {
        int u = ss.get<int>(0), v = ss.get<int>(1), l = ss.get<int>(2);
        eval.addEdge(u, v, l);
        ++num;
        if (num == FLAGS_batch_sz) {
            auto& input_mgr = eval.getInputMgr();
            GreedyAlg greedy(&input_mgr, FLAGS_budget);
            double val = greedy.run();
            oracle_calls += greedy.getOracleCalls();
            int graph_sz = input_mgr.getNumNodes();
            naive_oracle_calls += graph_sz * FLAGS_budget;

            rst.emplace_back(++t, val, oracle_calls, naive_oracle_calls,
                             graph_sz);

            eval.next();
            num = 0;

            printf("\t%5d\t%5.0f\t%6d\r", t, val, oracle_calls);
            fflush(stdout);
        }
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
        ioutils::saveTupleVec(rst, ofnm, true, "{}\t{:.2f}\t{}\t{}\t{}\n");
    }

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
