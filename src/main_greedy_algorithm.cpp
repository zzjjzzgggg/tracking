#include "stdafx.h"
#include "ram_graph_driver.h"
#include "hitting_time_analyzer.h"
#include "greedy_manager.h"
#include "walk_manager.h"

DEFINE_int32(budget, 10, "number of nodes to choose");

class Elem {
public:
    int step, node;
    double gain, time;  // chosen time in seconds

public:
    Elem(int step, int node, double gain)
        : step(step), node(node), gain(gain) {}

    void echo() const {
        if (step % 50 == 0)
            printf("i:%5d n:%8d g:%.2e t:%.2f\n", step, node, gain, time);
    }
};

void save(Config* conf, const vector<Elem>& chosen) {
#ifdef GAIN_AP
    string prefix = "AP";
#else
    string prefix = "HT";
#endif
    string out_fnm = osutils::join(
        conf->dir,
        "{}_B{}_W{}_T{}_D{}_wt{}.dat"_format(
            prefix, strutils::prettyNumber(FLAGS_budget), conf->walks,
            conf->hops, conf->depth, (int)conf->weight_strategy));
    auto out_ptr = ioutils::getIOOut(out_fnm);
    double rwd = 0;
    for (auto& e : chosen) {
        rwd += e.gain;
        out_ptr->save("{}\t{}\t{:.4e}\t{:.4e}\t{:.4f}\n"_format(
            e.step, e.node, e.gain, rwd, e.time));
    }
    printf("\nsaved to: %s\n\n", out_fnm.c_str());
}

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    osutils::Timer tm;

    Config conf;
    conf.info();

    // prepare walk manager
    vector<RAMGraphDriver> graph_drivers;
    graph_drivers.reserve(conf.n_cpus);
    RAMWalkStorage ram_store{&conf};
    WalkStepAnalyzer analyzer{&conf};
    Dispatcher dispatcher{&conf, &analyzer, &ram_store};
    GraphManager graph_mgr{&conf};
    WalkManager walk_mgr{&conf, &dispatcher};
    for (int core = 0; core < conf.n_cpus; core++) {
        graph_drivers.emplace_back(&conf, core);
        walk_mgr.addWalkEngine(&graph_drivers[core]);
        graph_mgr.add(&graph_drivers[core]);
    }

    GreedyManager greedy{&conf, &analyzer, &graph_mgr, &ram_store};

    auto cmp = [](Elem& a, Elem& b) { return a.gain < b.gain; };
    std::priority_queue<Elem, vector<Elem>, decltype(cmp)> pq(cmp);

    tm.tick();
    // initializing the queue
    for (int node = 0; node < conf.n_nodes; node++)
        pq.emplace(0, node, std::numeric_limits<double>::max());

    // greedily choosing nodes
    // 'max_gain' stors the maximum gain in current round
    int step = 1;
    double reward = 0, max_gain = 0;
    vector<Elem> chosen;
    chosen.reserve(FLAGS_budget);
    while (step <= FLAGS_budget) {
        Elem e = pq.top();
        pq.pop();
        int node = e.node;
        if (e.step == step) {
            e.time = tm.seconds();
            e.echo();
            chosen.push_back(e);
            ChosenSet::add(node);
            greedy.merge();
            step++;
            max_gain = 0;  // reset max gain
        } else {
            ChosenSet::add(node);
            walk_mgr.setWalksFrom(node, greedy.getWalksAt(node));
            walk_mgr.startWalking();
            e.gain = greedy.getGain(node);
            e.step = step;
            pq.push(e);
            if (e.gain > max_gain) {
                greedy.capture();
                max_gain = e.gain;
            }
            ChosenSet::remove(node);
            analyzer.clear();
            ram_store.clear();
        }
    }

    save(&conf, chosen);

    printf("cost time %s\n", tm.getStr().c_str());
    gflags::ShutDownCommandLineFlags();
    return 0;
}
