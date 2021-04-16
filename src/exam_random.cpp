/**
 * Copyright (C) by J.Z. (04/05/2018 19:28)
 * Distributed under terms of the MIT license.
 */
#ifndef DGRAPH
#include "dyn_bgraph_mgr.h"
#else
#include "dyn_dgraph_mgr.h"
#endif

#include "eval_stream.h"

#include <gflags/gflags.h>

DEFINE_string(graph, "", "input graph");
DEFINE_int32(budget, 10, "budget");
DEFINE_int32(end_tm, 100, "end time");
DEFINE_int32(batch_sz, 1, "batch size");
DEFINE_int32(repeat, 10, "repeat times");
DEFINE_int32(L, 10, "maximum lifetime");

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;
    rngutils::default_rng rng;

    std::vector<double> vals(FLAGS_end_tm, 0);

    printf("\ttime\tvalue\n");
    for (int rpt = 0; rpt < FLAGS_repeat; ++rpt) {
#ifndef DGRAPH
        EvalStream<DynBGraphMgr> eval(FLAGS_L);
#else
        EvalStream<DynDGraphMgr> eval(FLAGS_L);
#endif

        int t = 0, num = 0;
        ioutils::TSVParser ss(FLAGS_graph);
        while (ss.next()) {
            int u = ss.get<int>(0), v = ss.get<int>(1), l = ss.get<int>(2);
            eval.addEdge(u, v, l);
            ++num;
            if (num == FLAGS_batch_sz) {
                auto& input_mgr = eval.getInputMgr();
                auto samples =
                    rngutils::choose(input_mgr.getNodes(), FLAGS_budget, rng);
                double val = input_mgr.getReward(samples);
                vals[t++] += val;

                eval.next();
                num = 0;

                printf("\t%5d\t%5.0f\r", t, val);
                fflush(stdout);
            }
            if (t == FLAGS_end_tm) break;
        }
        printf("\n");
    }

    // averaging
    int t = 0;
    std::vector<std::pair<int, double>> rst;
    for (double val : vals) rst.emplace_back(++t, val / FLAGS_repeat);

    // save results
    std::string ofnm = strutils::insertMiddle(
        FLAGS_graph, "random_k{}"_format(FLAGS_budget), "dat");
    std::string ano =
        "#graph: {}\nbudget: {}\n#batch size: {}\n#end time: {}\n"_format(
            FLAGS_graph, FLAGS_budget, FLAGS_batch_sz, FLAGS_end_tm);
    ioutils::savePrVec(rst, ofnm, "{}\t{:.1f}\n", ano,true);

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
