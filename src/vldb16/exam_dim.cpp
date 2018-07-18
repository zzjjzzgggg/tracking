#include "../stdafx.h"

#ifndef DGRAPH
#include "../dyn_bgraph_mgr.h"
#else
#include "../dyn_dgraph_mgr_v2.h"
#endif

#include "dim_stream.h"

#include <gflags/gflags.h>

DEFINE_string(graph, "", "input graph");
DEFINE_int32(budget, 10, "budget");
DEFINE_int32(end_tm, 10000, "end time");
DEFINE_int32(L, 10000, "maximum lifetime");
DEFINE_int32(beta, 32, "beta");

int main(int argc, char *argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

#ifndef DGRAPH
    DIMStream<DynBGraphMgr> dim(FLAGS_L, FLAGS_beta);
#else
    DIMStream<DynDGraphMgr> dim(FLAGS_L, FLAGS_beta);
#endif

    std::vector<int> seeds;

    printf("\t%-12s\n", "time");
    int t = 0;
    ioutils::TSVParser ss(FLAGS_graph);
    while (ss.next()) {
        int u = ss.get<int>(0), v = ss.get<int>(1), l = ss.get<int>(2);
        dim.addEdge(u, v, l);
        ++t;

        seeds = dim.infmax(FLAGS_budget);

        printf("\t%-12d\r", t);
        fflush(stdout);

        if (t == FLAGS_end_tm) break;
        dim.next();
    }
    printf("\n");

    auto input_mgr = dim.getInputMgr();
    double inf = input_mgr.getReward(seeds);
    printf("inf: %.2f\n", inf);

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
