#include "stdafx.h"
#include "ram_graph_driver.h"
#include "hitting_time_analyzer.h"
#include "greedy_manager.h"
#include "walk_manager.h"

DEFINE_string(node_gain, "", "groundtruth node-gain vector filename");
DEFINE_int32(repeat, 5, "repeat times");
DEFINE_int32(trials, 4, "trials");

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

    Config conf;

    // prepare walk manager
    vector<RAMGraphDriver> graph_drivers;
    graph_drivers.reserve(conf.n_cpus);
    WalkStepAnalyzer analyzer{&conf};
    Dispatcher dispatcher{&conf, &analyzer};
    GraphManager graph_mgr{&conf};
    WalkManager walk_mgr{&conf, &dispatcher};
    for (int core = 0; core < conf.n_cpus; core++) {
        graph_drivers.emplace_back(&conf, core);
        walk_mgr.addWalkEngine(&graph_drivers[core]);
        graph_mgr.add(&graph_drivers[core]);
    }

    // load node-gain pairs
    vector<std::pair<int, double>> node_gain_vec;
    ioutils::loadPrVec(osutils::join(conf.dir, FLAGS_node_gain), node_gain_vec);
    int num_samples = node_gain_vec.size();

    tm.tick();
    double ratio = 0, nrmse = 0;
    for (int trial = 0; trial < FLAGS_trials; trial++) {
        conf.setWalksDir(fmt::format(FLAGS_walks_dir, trial));
        GreedyManager greedy{&conf, &analyzer, &graph_mgr};
        for (int i = 0; i < num_samples; i++) {
            int s = node_gain_vec[i].first;
            double truth = node_gain_vec[i].second;
            ChosenSet::add(s);
            for (int rpt = 0; rpt < FLAGS_repeat; rpt++) {
                analyzer.clear();
                walk_mgr.setWalksFrom(s, greedy.getWalksAt(s));
                walk_mgr.startWalking();
                double r = greedy.getGain(s) / truth;
                ratio += r;
                nrmse += std::pow(r - 1, 2);
            }
            ChosenSet::remove(s);
        }
    }
    ratio /= (FLAGS_trials * FLAGS_repeat * num_samples);
    nrmse = std::sqrt(nrmse / (FLAGS_trials * FLAGS_repeat * num_samples));
    double secs = tm.seconds() / (FLAGS_trials * FLAGS_repeat * num_samples);

    printf("%d\t%d\t%.4f\t%.4f\t%.4f\n", conf.walks, conf.depth, nrmse, ratio,
           secs);

    // printf("# cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
