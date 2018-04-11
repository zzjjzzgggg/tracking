/**
 * Copyright (C) by J.Z. (04/05/2018 19:28)
 * Distributed under terms of the MIT license.
 */
#include "dyn_bgraph_mgr.h"
#include "basic_reduction.h"

#include <gflags/gflags.h>

DEFINE_string(graph, "", "input bipartite graph (user, venue, ...)");
DEFINE_int32(budget, 50, "budget");
DEFINE_int32(end_tm, 100, "end time");
DEFINE_int32(batch_sz, 10, "batch size");
DEFINE_int32(L, 10, "maximum lifetime");
DEFINE_double(eps, 0.2, "epsilon");

int main(int argc, char *argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

    BasicReduction<DynBGraphMgr> basic(FLAGS_L, FLAGS_budget, FLAGS_eps);

    std::vector<std::pair<int, double>> tm_val;

    std::vector<int> S;
    int t = 0, num = 0;
    ioutils::TSVParser ss(FLAGS_graph);
    while (ss.next()) {
        int u = ss.get<int>(0), v = ss.get<int>(1), l = ss.get<int>(2);
        basic.addEdge(u, v, l);
        ++num;
        if (num == FLAGS_batch_sz) {
            basic.update();
            auto rst = basic.getResult();
            double val = rst.second;
            S = basic.getSolution(rst.first);
            basic.clear();
            num = 0;

            printf("\t%d\t\t%.0f\r", ++t, val);
            fflush(stdout);
            tm_val.emplace_back(t, val);
        }
        if (t == FLAGS_end_tm) break;
    }
    printf("\n");

    ioutils::saveVec(S, "S_basic.dat");
    // save results
    std::string ofnm = strutils::insertMiddle(
        FLAGS_graph, "basic_k{}e{:g}"_format(FLAGS_budget, FLAGS_eps), "dat");
    std::string ano = fmt::format(
        "#graph: {}\n#budget: {}\n#batch size: {}\n#end time: {}\n#epsilon: "
        "{:.2f}\n",
        FLAGS_graph, FLAGS_budget, FLAGS_batch_sz, FLAGS_end_tm, FLAGS_eps);
    ioutils::savePrVec(tm_val, ofnm, true, "{:d}\t{:.1f}\n", ano);

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
