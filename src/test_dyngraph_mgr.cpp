/**
 * Copyright (C) by J.Z. (04/04/2018 18:12)
 * Distributed under terms of the MIT license.
 */

#include <gflags/gflags.h>

#include "dyngraph_mgr.h"


int main(int argc, char *argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

    // std::string gfn = "/dat/workspace/datasets/cit-HepTh_wcc_digraph_mapped.gz";
    std::string gfn = "/home/jzzhao/workspace/streaming/test_graph.txt";

    dir::DGraph graph = loadEdgeList<dir::DGraph>(gfn);
    DirBFS bfs(graph);

    auto edges = ioutils::loadPrVec<int, int>(gfn);

    DynGraphMgr mgr(12);
    for (auto& e: edges) mgr.addEdge(e.first, e.second);
    printf("edge added\n");

    mgr.updateDAG();

    printf("nd\ttruth (tm)\t\test (tm)\t\t err\n");
    for (int i = 0; i < 10; i++) {
        int nd = graph.sampleNode();

        tm.tick();
        bfs.doBFS(nd);
        int truth = bfs.getBFSTreeSize();
        double t1 = tm.seconds();

        tm.tick();
        double est = mgr.estimate(nd);
        double t2 = tm.seconds();

        printf("%d\t%d (tm %.3f)\t\t%.0f (tm %.3f)\t\t%.2f%%\n",
               nd, truth, t1, est, t2, std::abs(est - truth) / truth * 100);
    }

    mgr.reset();

    tm.tick();
    mgr.addEdge(1, 12);
    double t1 = tm.seconds();

    tm.tick();
    auto vec = mgr.updateDAG();
    double t2 = tm.seconds();
    printf("%.4f, %.4f, %lu\n", t1, t2, vec.size());


    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
