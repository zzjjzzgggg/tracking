/**
 * Copyright (C) by J.Z. (04/05/2018 19:28)
 * Distributed under terms of the MIT license.
 */
#include "dyn_bgraph_mgr.h"
#include "eval_stream.h"

#include <gflags/gflags.h>

DEFINE_string(graph, "", "input bipartite graph (user, venue, ...)");
DEFINE_string(nodes, "", "nodes");
DEFINE_int32(end_tm, 100, "end time");
DEFINE_int32(L, 10, "maximum lifetime");

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

    EvalStream<DynBGraphMgr> eval(FLAGS_L);

    int t = 0;
    ioutils::TSVParser ss(FLAGS_graph);
    while (ss.next()) {
        int u = ss.get<int>(0), v = ss.get<int>(1), l = ss.get<int>(2);
        eval.addEdge(u, v, l);
        ++t;
        if (t == FLAGS_end_tm) break;
    }

    auto nodes = ioutils::loadVec<int>(FLAGS_nodes);
    double val = eval.eval(nodes);
    printf("val: %.2f\n", val);


    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
