/**
 * Copyright (C) by J.Z. (04/05/2018 19:28)
 * Distributed under terms of the MIT license.
 */
#ifndef DGRAPH
#include "dyn_bgraph_mgr.h"
#else
#include "dyn_dgraph_mgr.h"
#endif

#include "sieve_adn.h"
#include <gflags/gflags.h>

DEFINE_string(graph, "", "input bipartite graph (user, venue, ...)");
DEFINE_int32(budget, 10, "budget");
DEFINE_int32(end_tm, 100, "end time");
DEFINE_int32(batch_sz, 1, "batch size");
DEFINE_double(eps, 0.2, "epsilon");

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

    printf("\ttime\tvalue\tocalls\tn-th\taff-nd\n");

    int t = 0, num = 0, n = 0, ocalls = 0;
    ioutils::TSVParser ss(FLAGS_graph);
    while (ss.next()) {
        adn.feedEdge(ss.get<int>(0), ss.get<int>(1));
        if (++num == FLAGS_batch_sz) {
            n += adn.update();
            ocalls += adn.getOracleCalls();
            double val = adn.getResult().second;
            rst.emplace_back(++t, val, ocalls);

            adn.clear();
            num = 0;

            printf("\t%d\t%.0f\t%6d\t%3d\t%5d\r", t, val, ocalls,
                   adn.getNumThresholds(), n);
            fflush(stdout);
        }
        if (t == FLAGS_end_tm) break;
    }
    printf("\n");

    // save results
    std::string ofnm = strutils::insertMiddle(
        FLAGS_graph, "adn_k{}e{:g}"_format(FLAGS_budget, FLAGS_eps), "dat");
    std::string ano = fmt::format(
        "#graph: {}\n#budget: {}\n#batch size: {}\n#end time: {}\n#epsilon: "
        "{:.2f}\n",
        FLAGS_graph, FLAGS_budget, FLAGS_batch_sz, FLAGS_end_tm, FLAGS_eps);
    ioutils::saveTupleVec(rst, ofnm, true, "{}\t{:.1f}\t{}\n", ano);

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
