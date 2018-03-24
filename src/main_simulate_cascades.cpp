/*
 * Copyright (C) 2017 by J. Zhao
 * Distributed under terms of the MIT license.
 */

#include "stdafx.h"
#include <gflags/gflags.h>

DEFINE_string(root, "", "work directory");
DEFINE_int32(cpus, std::thread::hardware_concurrency(), "number of cores");
DEFINE_double(p, 0.9, "continue probability");
DEFINE_double(cascades, 10000, "number of cascades");
DEFINE_bool(rw, false, "sim RW?");

void simRW(const graph::DGraph* G, const int num, const int cpu) {
    string out_fnm = osutils::join(
        FLAGS_root, "casd_rw_{}_{:.1f}_{}.gz"_format(
                        strutils::prettyNumber(FLAGS_cascades), FLAGS_p, cpu));
    auto po = ioutils::getIOOut(out_fnm);
    randutils::default_rng rng;
    const int UNIT = num / 10, cid_st = cpu * num;
    for (int iter = 0; iter < num; iter++) {
        if ((iter + 1) % UNIT == 0)
            printf("[%d]: %d\n", cpu, (iter + 1) / UNIT);
        int u = G->sampleNode(), casd = iter + cid_st, t = 0;
        po->save("{}\t{}\t{}\n"_format(casd, u, t++));
        while (rng.uniform() < FLAGS_p) {
            u = G->sampleOutNbr(u);
            po->save("{}\t{}\t{}\n"_format(casd, u, t++));
            auto&& nd_obj = G->getNode(u);
            if (nd_obj.getOutDeg() == 1 && nd_obj.getOutNbr(0) == u) break;
        }
    }
    printf("saved to %s\n", out_fnm.c_str());
}

void simIC(const graph::DGraph* G, const int num, const int cpu) {
    string out_fnm = osutils::join(
        FLAGS_root, "casd_ic_{}_{:g}_{}.gz"_format(
                        strutils::prettyNumber(FLAGS_cascades), FLAGS_p, cpu));
    auto po = ioutils::getIOOut(out_fnm);
    randutils::default_rng rng;
    const int UNIT = num / 10, cid_st = cpu * num;

    unordered_set<int> infected;  // node -> time
    vector<vector<int>> infectious(2);

    for (int iter = 0; iter < num; iter++) {
        if ((iter + 1) % UNIT == 0)
            printf("[%d]: %d\n", cpu, (iter + 1) / UNIT);
        int u = G->sampleNode(), casd = iter + cid_st, t = 0;
        po->save("{}\t{}\t{}\n"_format(casd, u, t));
        infected.clear();
        infectious[0].clear();
        infectious[1].clear();
        infected.insert(u);
        infectious[0].push_back(u);
        while (true) {
            auto &rvec = infectious[t % 2], &wvec = infectious[++t % 2];
            if (rvec.empty()) break;
            wvec.clear();
            for (int seed : rvec) {
                auto& nd_obj = G->getNode(seed);
                int deg = nd_obj.getOutDeg();
                for (int d = 0; d < deg; d++) {
                    int nbr = nd_obj.getOutNbr(d);
                    if (infected.find(nbr) == infected.end() &&
                        rng.uniform() < FLAGS_p) {
                        infected.insert(nbr);
                        wvec.push_back(nbr);
                        po->save("{}\t{}\t{}\n"_format(casd, nbr, t));
                    }
                }
            }
        }
    }
    printf("saved to %s\n", out_fnm.c_str());
}

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

    string gfnm = osutils::join(FLAGS_root, "graph_bin.lz");
    graph::DGraph G = graph::loadBinary<graph::DGraph>(gfnm);
    printf("n: %d, e: %d\n", G.getNodes(), G.getEdges());

    int num_per_cpu = FLAGS_cascades / FLAGS_cpus;
    vector<std::future<void>> futures;
    for (int cpu = 0; cpu < FLAGS_cpus; cpu++) {
        if (FLAGS_rw) {
            futures.push_back(
                std::async(std::launch::async, &simRW, &G, num_per_cpu, cpu));
        } else {
            futures.push_back(
                std::async(std::launch::async, &simIC, &G, num_per_cpu, cpu));
        }
    }
    for (auto& future : futures) future.get();

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
