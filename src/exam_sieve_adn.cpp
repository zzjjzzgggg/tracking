/**
 * Copyright (C) by J.Z. (04/05/2018 19:28)
 * Distributed under terms of the MIT license.
 */
#ifndef DGRAPH
#include "dyn_bgraph_mgr.h"
#else
#include "dyn_dgraph_mgr_v2.h"
#endif

#include "sieve_adn.h"
#include <gflags/gflags.h>

DEFINE_string(graph, "", "input bipartite graph (user, venue, ...)");
DEFINE_int32(budget, 10, "budget");
DEFINE_int32(end_tm, 100, "end time");
DEFINE_int32(batch, 1, "batch size");
DEFINE_double(eps, 0.2, "epsilon");
DEFINE_bool(save, true, "save results or not");

int main(int argc, char *argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

#ifndef DGRAPH
    SieveADN<DynBGraphMgr> adn{FLAGS_budget, FLAGS_eps};
#else
    SieveADN<DynDGraphMgr> adn{FLAGS_budget, FLAGS_eps};
#endif

    std::vector<std::tuple<int, double, int>> rst;

    printf("\t%-12s%-12s%-12s%-12s%-12s\n", "time", "value", "#calls",
           "#thresholds", "n");

    int t = 0, num = 0, n = 0, ocalls = 0;
    ioutils::TSVParser ss(FLAGS_graph);
    while (ss.next()) {
        adn.feedEdge(ss.get<int>(0), ss.get<int>(1));
        if (++num < FLAGS_batch) continue;
        t += num;
        num = 0;

        n += adn.update();
        ocalls += adn.getOracleCalls();
        double val = adn.getResult().second;
        rst.emplace_back(t, val, ocalls);

        adn.clear();

        printf("\t%-12d%-12.0f%-12d%-12d%-12d\r", t, val, ocalls,
               adn.getNumThresholds(), n);
        fflush(stdout);
        if (t == FLAGS_end_tm) break;
    }
    printf("\n");

    if (FLAGS_save) {
        // save results
        std::string ofnm = strutils::insertMiddle(
            FLAGS_graph, "adn_k{}e{:g}"_format(FLAGS_budget, FLAGS_eps), "dat");
        std::string ano = fmt::format(
            "#graph: {}\n#budget: {}\n#batch size: {}\n#end time: "
            "{}\n#epsilon: "
            "{:.2f}\n",
            FLAGS_graph, FLAGS_budget, FLAGS_batch, FLAGS_end_tm, FLAGS_eps);
        ioutils::saveTupleVec(rst, ofnm, true, "{}\t{:.1f}\t{}\n", ano);
    }

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
