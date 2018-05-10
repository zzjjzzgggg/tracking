/**
 * Copyright (C) by J.Z. (04/05/2018 19:28)
 * Distributed under terms of the MIT license.
 */
#ifndef DGRAPH
#include "dyn_bgraph_mgr.h"
#else
#include "dyn_dgraph_mgr.h"
#endif

#include "basic_reduction.h"

#include <gflags/gflags.h>

DEFINE_string(graph, "", "input graph");
DEFINE_int32(budget, 50, "budget");
DEFINE_int32(end_tm, 100, "end time");
DEFINE_int32(batch_sz, 1, "batch size");
DEFINE_int32(L, 10, "maximum lifetime");
DEFINE_double(eps, 0.2, "epsilon");
DEFINE_bool(save, true, "save results or not");

int main(int argc, char *argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

#ifndef DGRAPH
    BasicReduction<DynBGraphMgr> basic(FLAGS_L, FLAGS_budget, FLAGS_eps);
#else
    BasicReduction<DynDGraphMgr> basic(FLAGS_L, FLAGS_budget, FLAGS_eps);
#endif

    std::vector<std::tuple<int, double, int>> rst;

    printf("\ttime\tval\tocals\n");
    int t = 0, num = 0, calls = 0;
    ioutils::TSVParser ss(FLAGS_graph);
    while (ss.next()) {
        int u = ss.get<int>(0), v = ss.get<int>(1), l = ss.get<int>(2);
        basic.feedEdge(u, v, l);
        if (++num == FLAGS_batch_sz) {
            basic.update();
            calls += basic.statOracleCalls();
            double val = basic.getResult().second;

            basic.next();
            num = 0;

            rst.emplace_back(++t, val, calls);

            printf("\t%5d\t%5.0f\t%6d\r", t, val, calls);
            fflush(stdout);
        }
        if (t == FLAGS_end_tm) break;
    }
    printf("\n");

    // save results
    if (FLAGS_save) {
        std::string ofnm = strutils::insertMiddle(
            FLAGS_graph,
            "basic_K{}T{}e{:g}"_format(
                FLAGS_budget, strutils::prettyNumber(FLAGS_end_tm), FLAGS_eps),
            "dat");
        ioutils::saveTripletVec(rst, ofnm, true, "{}\t{:.2f}\t{}\n");
    }

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
