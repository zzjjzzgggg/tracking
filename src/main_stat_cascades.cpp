/**
 * Copyright (C) 2017 by J. Zhao
 * Distributed under terms of the MIT license.
 */

#include "stdafx.h"
#include <gflags/gflags.h>

DEFINE_string(G, "graph_bin.lz", "graph file name");
DEFINE_int32(cpus, std::thread::hardware_concurrency(), "number of cores");
DEFINE_int32(B, 0, "budget");
DEFINE_string(C, "", "cascades data");
DEFINE_string(S, "", "S");
DEFINE_int32(col, 0, "column to read");
DEFINE_double(wt, 5, "weight from a sensor to target n");

void updateMap(const DGraph& G, const int c, const int s, const int t,
               randutils::default_rng& rng, unordered_map<int, int>& ct) {
    int deg = G.getNode(s).getOutDeg();
    double psn = FLAGS_wt / (FLAGS_wt + deg);
    if (rng.uniform() < psn) {
        if (ct.find(c) != ct.end())
            ct[c] = std::min(ct[c], t);
        else
            ct[c] = t;
    }
}

double mean(const unordered_map<int, int>& ct) {
    double sum = 0;
    for (auto& pr : ct) sum += pr.second;
    return sum / ct.size();
}

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

    randutils::default_rng rng;
    DGraph G = loadBinary<DGraph>(FLAGS_G);

    vector<int> S;
    ioutils::loadVec(FLAGS_S, S, FLAGS_col);

    assert_msg(S.size() >= FLAGS_B * 3, "sensor size: %lu < 3 x %d", S.size(),
               FLAGS_B);

    unordered_set<int> S0(S.begin(), S.begin() + FLAGS_B),
        S1(S.begin(), S.begin() + FLAGS_B * 2),
        S2(S.begin(), S.begin() + FLAGS_B * 3);
    unordered_map<int, int> ct0, ct1, ct2;
    double cov[] = {0, 0, 0}, delay[] = {0, 0, 0};

    for (int rpt = 0; rpt < 10; rpt++) {
        for (int cpu = 0; cpu < FLAGS_cpus; cpu++) {
            string fnm = fmt::format(FLAGS_C, cpu);
            if (!osutils::exists(fnm)) continue;
            ioutils::TSVParser ss(fnm);
            while (ss.next()) {
                int c = ss.get<int>(0), u = ss.get<int>(1), t = ss.get<int>(2);
                if (S0.find(u) != S0.end()) updateMap(G, c, u, t, rng, ct0);
                if (S1.find(u) != S1.end()) updateMap(G, c, u, t, rng, ct1);
                if (S2.find(u) != S2.end()) updateMap(G, c, u, t, rng, ct2);
            }
        }
        cov[0] += ct0.size();
        cov[1] += ct1.size();
        cov[2] += ct2.size();
        delay[0] += mean(ct0);
        delay[1] += mean(ct1);
        delay[2] += mean(ct2);
    }

    printf("%lu\t%.3f\t%.3f\n", S0.size(), cov[0] / 10, delay[0] / 10);
    printf("%lu\t%.3f\t%.3f\n", S1.size(), cov[1] / 10, delay[1] / 10);
    printf("%lu\t%.3f\t%.3f\n", S2.size(), cov[2] / 10, delay[2] / 10);

    // printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
