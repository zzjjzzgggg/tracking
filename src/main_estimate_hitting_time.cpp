#include "stdafx.h"
#include "walk_engine.h"
#include "ram_graph_driver.h"
#include "ram_walk_storage.h"
#include "hitting_time_analyzer.h"

DEFINE_int32(trials, 10, "trials");

int main(int argc, char* argv[]) {
    FLAGS_cores = 1;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

    Config conf;
    conf.info();

    // ChosenSet::setAllConnection();
    ChosenSet::add(4228);

    RAMGraphDriver graph_driver{&conf};

    vector<std::future<void>> futures;
    for (int trial = 0; trial < FLAGS_trials; trial++) {
        futures.push_back(
            std::async(std::launch::async,
                       [&](int trial) {
                           HittingTimeAnalyzer analyzer{&conf};
                           Dispatcher dispatcher{&conf, &analyzer};
                           WalkEngine walk_engine{&conf, &graph_driver};
                           walk_engine.registerDispatcher(0, &dispatcher);
                           walk_engine.initWalks();
                           walk_engine.startWalking();
                           analyzer.saveResult(trial);
                       },
                       trial));
    }
    for (auto& f : futures) f.get();

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
