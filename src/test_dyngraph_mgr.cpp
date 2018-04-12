/**
 * Copyright (C) by J.Z. (04/04/2018 18:12)
 * Distributed under terms of the MIT license.
 */

#include <gflags/gflags.h>

#include "dyn_dgraph_mgr.h"

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

    // std::string gfn =
    // "/dat/workspace/dat/cit-HepTh_wcc_digraph_mapped.gz";
    std::string gfn = "/home/jzzhao/workspace/streaming/test_graph.txt";

    dir::DGraph graph = loadEdgeList<dir::DGraph>(gfn);
    DirBFS<dir::DGraph> bfs(graph);

    auto edges = ioutils::loadPrVec<int, int>(gfn);

    DynDGraphMgr mgr(12);
    for (auto& e : edges) mgr.addEdge(e.first, e.second);
    printf("edge added\n");

    mgr.updateDAG();

    printf("nd\ttruth\ttm\test\ttm\terr(%%)\n");
    for (auto ni = graph.beginNI(); ni != graph.endNI(); ni++) {
        int nd = ni->first;

        tm.tick();
        bfs.doBFS(nd);
        int truth = bfs.getBFSTreeSize();
        double t1 = tm.seconds();

        tm.tick();
        double est = mgr.estimate(nd);
        double t2 = tm.seconds();

        double err = std::abs(est - truth) / truth * 100;
        printf("%d\t%d\t%.1e\t%.2f\t%.1e\t%.2f\n", nd, truth, t1, est, t2, err);
    }

    tm.tick();
    mgr.addEdge(1, 4);
    double t1 = tm.seconds();

    tm.tick();
    auto vec = mgr.updateDAG();
    double t2 = tm.seconds();
    printf("%.4f, %.4f, %lu\n", t1, t2, vec.size());

    graph.addEdge(1, 4);

    for (auto ni = graph.beginNI(); ni != graph.endNI(); ni++) {
        int nd = ni->first;

        tm.tick();
        bfs.doBFS(nd);
        int truth = bfs.getBFSTreeSize();
        double t1 = tm.seconds();

        tm.tick();
        double est = mgr.estimate(nd);
        double t2 = tm.seconds();

        double err = std::abs(est - truth) / truth * 100;
        printf("%d\t%d\t%.1e\t%.2f\t%.1e\t%.2f\n", nd, truth, t1, est, t2, err);
    }

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
