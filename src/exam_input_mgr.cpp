/**
 * Copyright (C) by J.Z. (2018-07-12 18:20)
 * Distributed under terms of the MIT license.
 */

#include "stdafx.h"
#ifndef DGRAPH
#include "dyn_bgraph_mgr.h"
#else
#include "dyn_dgraph_mgr_v2.h"
#endif

#include <gflags/gflags.h>

DEFINE_string(graph, "", "input graph");
DEFINE_int32(end_tm, 100, "end time");
DEFINE_int32(batch, 1, "batch size");

void evalGetAffectedNodes() {
#ifndef DGRAPH
    DynBGraphMgr input_mgr;
#else
    DynDGraphMgr input_mgr;
#endif

    int t = 0, n = 0;
    ioutils::TSVParser ss(FLAGS_graph);
    while (ss.next()) {
        int u = ss.get<int>(0), v = ss.get<int>(1), l = ss.get<int>(2);
        input_mgr.addEdge(u, v);
        if (++n < FLAGS_batch) continue;
        n = 0;
        t += FLAGS_batch;

        auto nodes = input_mgr.getAffectedNodes();

        printf("\t%-12d%-12lu\r", t, nodes.size());
        fflush(stdout);

        if (t == FLAGS_end_tm) break;
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

    evalGetAffectedNodes();

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
