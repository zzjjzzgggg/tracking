/**
 * Copyright (C) by J.Z. (04/05/2018 19:28)
 * Distributed under terms of the MIT license.
 */
#ifndef DGRAPH
#include "dyn_bgraph_mgr.h"
#else
#include "dyn_dgraph_mgr_v2.h"
#endif

#include "eval_stream.h"
// #include "greedy_alg.h"

#include <gflags/gflags.h>

DEFINE_string(graph, "", "input graph");
DEFINE_string(nodes, "", "nodes");
DEFINE_int32(budget, 10, "budget");
DEFINE_int32(end_tm, 10000, "end time");
DEFINE_int32(L, 10000, "maximum lifetime");
DEFINE_bool(decay, true, "decay or nodecay");
DEFINE_bool(save_edges, false, "save edges to file");
DEFINE_bool(eval_nodes, false, "evaluate the influence of a set of nodes");

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

#ifndef DGRAPH
    EvalStream<DynBGraphMgr> eval(FLAGS_L);
#else
    EvalStream<DynDGraphMgr> eval(FLAGS_L);
#endif

    int t = 0;
    ioutils::TSVParser ss(FLAGS_graph);
    if (FLAGS_decay) {
        while (ss.next()) {
            int u = ss.get<int>(0), v = ss.get<int>(1), l = ss.get<int>(2);
            eval.addEdge(u, v, l);
            if (++t == FLAGS_end_tm) break;
            eval.next();
        }
    } else {
        while (ss.next()) {
            int u = ss.get<int>(0), v = ss.get<int>(1);
            eval.addEdge(u, v);
            if (++t == FLAGS_end_tm) break;
        }
    }

    if (FLAGS_save_edges) {
        std::string filename = strutils::insertMiddle(
            FLAGS_graph, "T{}"_format(strutils::prettyNumber(FLAGS_end_tm)),
            "edges");
        eval.saveEdges(filename);
    }

    if (FLAGS_eval_nodes) {
        auto input_mgr = eval.getInputMgr();
        auto nodes = ioutils::loadVec<int>(FLAGS_nodes);
        double rwd = input_mgr.getReward(nodes);
        printf("getReward: %.4f\n", rwd);
    }

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
