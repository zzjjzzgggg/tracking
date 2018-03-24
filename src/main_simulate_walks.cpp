#include "stdafx.h"
#include "walk_engine.h"
#include "ram_graph_driver.h"
#include "disk_walk_storage.h"

int main(int argc, char *argv[]) {
    FLAGS_cores = 1;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

    Config conf;
    conf.info();

    RAMGraphDriver driver{&conf};
    DiskWalkStorage storage{&conf, true};
    Dispatcher dispatcher{&conf, nullptr, &storage};

    WalkEngine walk_engine{&conf, &driver};
    walk_engine.registerDispatcher(0, &dispatcher);
    walk_engine.initWalks();
    walk_engine.startWalking();

    printf("sorting buckets...\n");
    for (int bkt = 0; bkt < conf.n_bkts; bkt++) storage.sortBucket(bkt);

    printf("walks saved to %s\n", conf.walks_dir.c_str());

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
