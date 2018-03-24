#include "stdafx.h"
#include "config.h"

DEFINE_int32(task_id, -1, "0: lz4; 1: split; 2: test; -1: nothing");
DEFINE_string(output, "", "outinput filename");

void test_graph() {
    int pre = -1, mn = 1000000, mx = -1;
    std::unordered_set<int> nodes;
    ioutils::TSVParser ss(FLAGS_graph);
    while (ss.next()) {
        int src = ss.get<int>(0), dst = ss.get<int>(1);
        assert_msg(src >= pre, "%d >= %d", src, pre);
        pre = src;
        nodes.insert(src);
        nodes.insert(dst);
        mn = std::min(mn, std::min(src, dst));
        mx = std::max(mx, std::max(src, dst));
    }
    assert_msg(mn == 0, "min node id: %d", mn);
    assert_msg(mx == nodes.size() - 1, "max node id: %d", mx);
    printf("graph is OK.\n");
}

void toLZ4() {
    ioutils::LZ4Out lzo(FLAGS_output.c_str());
    ioutils::TSVParser ss(FLAGS_graph);
    while (ss.next()) {
        int src = ss.get<int>(0), dst = ss.get<int>(1);
        lzo.save(src);
        lzo.save(dst);
    }
    printf("LZ4 file saved to %s\n", FLAGS_output.c_str());
}

void split(Config* conf) {
    ioutils::LZ4Out lzo;
    int pre_id = -1;
    ioutils::TSVParser ss(conf->graph_fnm);
    while (ss.next()) {
        int src = ss.get<int>(0), dst = ss.get<int>(1);
        int core_id = src / conf->n_nodes_a_cpu;
        if (core_id != pre_id) {
            pre_id = core_id;
            string filename = fmt::format(FLAGS_output, core_id);
            lzo.open(filename.c_str());
            printf("new file: %s\n", filename.c_str());
        }
        lzo.save(src);
        lzo.save(dst);
    }
}

int main(int argc, char* argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

    Config conf;
    conf.info();

    switch (FLAGS_task_id) {
        case 0:
            toLZ4();
            break;
        case 1:
            split(&conf);
            break;
        case 2:
            test_graph();
            break;
        default:
            break;
    }

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
