#include "stdafx.h"
#include "ram_graph_driver.h"
#include "disk_walk_storage.h"
#include "walk_manager.h"

int main(int argc, char *argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

    Config conf;
    conf.info();

    vector<RAMGraphDriver> drivers;
    drivers.reserve(conf.n_cpus);

    DiskWalkStorage storage{&conf, true};

    Dispatcher dispatcher{&conf, nullptr, &storage};

    WalkManager walk_mgr{&conf, &dispatcher};
    for (int core = 0; core < conf.n_cpus; core++) {
        drivers.emplace_back(&conf, core);
        walk_mgr.addWalkEngine(&drivers[core]);
    }

    walk_mgr.initWalks();
    walk_mgr.startWalking();

    printf("sorting buckets...\n");
    for (int bkt = 0; bkt < conf.n_bkts; bkt++) storage.sortBucket(bkt);

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
