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

int main(int argc, char *argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

#ifndef DGRAPH
    DIMStream<DynBGraphMgr > dim(FLAGS_L);
#else
    DIMStream<DynDGraphMgr> dim(FLAGS_L);
#endif

    printf("\t%-12s%-12s%-12s\n", "time", "hist", "dim");
    int t = 0;
    ioutils::TSVParser ss(FLAGS_graph);
    while (ss.next()) {
        int u = ss.get<int>(0), v = ss.get<int>(1), l = ss.get<int>(2);
        dim.addEdge(u, v, l);
        ++t;

        auto nodes = dim.infmax(FLAGS_budget);
        auto input_mgr = dim.getInputMgr();
        double inf = input_mgr.getReward(nodes);
        double infest = dim.infest(nodes);

        printf("\t%-12d%-12.0f%-12.0f\r", t, inf, infest);
        fflush(stdout);

        if (t == FLAGS_end_tm) break;
        dim.next();
    }
    printf("\n");

    // auto nodes = dim.infmax(FLAGS_budget);
    // ioutils::saveVec(nodes, "dim_seeds_exam.dat");


    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
