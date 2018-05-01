/**
 * Copyright (C) by J.Z. (04/05/2018 19:28)
 * Distributed under terms of the MIT license.
 */
#ifndef DGRAPH
#include "dyn_bgraph_mgr.h"
#else
#include "dyn_dgraph_mgr.h"
#endif

#include "hist_approx.h"

#include <gflags/gflags.h>

DEFINE_string(graph, "", "input graph");
DEFINE_int32(budget, 10, "budget");
DEFINE_int32(end_tm, 100, "end time");
DEFINE_int32(batch_sz, 10, "batch size");
DEFINE_int32(L, 10, "maximum lifetime");
DEFINE_double(eps, 0.2, "epsilon");
DEFINE_bool(save, true, "save results or not");

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

#ifndef DGRAPH
    HistApprox<DynBGraphMgr> hist(FLAGS_L, FLAGS_budget, FLAGS_eps);
#else
    HistApprox<DynDGraphMgr> hist(FLAGS_L, FLAGS_budget, FLAGS_eps);
#endif

    std::vector<std::tuple<int, double, int>> rst;
    std::unordered_map<int, std::vector<IntPr>> edge_batch;

    printf("\ttime\tval\tocals\tn_algs\n");
    int t = 0, num = 0, ocalls = 0;
    ioutils::TSVParser ss(FLAGS_graph);
    while (ss.next()) {
        int u = ss.get<int>(0), v = ss.get<int>(1), l = ss.get<int>(2);
        edge_batch[l].emplace_back(u, v);
        if (++num == FLAGS_batch_sz) {
            for (auto& pr : edge_batch) hist.feedAndUpdate(pr.second, pr.first);

            double val = hist.getResult().second;
            for (auto& pr : edge_batch)
                if (pr.first > 1) hist.bufEdges(pr.second, pr.first);

            ocalls += hist.statOracleCalls();
            rst.emplace_back(++t, val, ocalls);

            edge_batch.clear();
            hist.next();
            num = 0;

            printf("\t%5d\t%5.0f\t%6d\t%3d\r", t, val, ocalls,
                   hist.getNumAlgs());
            fflush(stdout);
        }
        if (t == FLAGS_end_tm) break;
    }
    printf("\n");

    // save results
    if (FLAGS_save) {
        std::string ofnm = strutils::insertMiddle(
            FLAGS_graph,
            "hist_K{}T{}e{:g}"_format(
                FLAGS_budget, strutils::prettyNumber(FLAGS_end_tm), FLAGS_eps),
            "dat");
        ioutils::saveTripletVec(rst, ofnm, true, "{}\t{:.2f}\t{}\n");
    }

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
