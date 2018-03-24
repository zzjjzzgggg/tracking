#include "greedy_manager.h"

void GreedyManager::getWalkResumingHop(const int node,
                                       const vector<long>& walks,
                                       const bool from_disk,
                                       unordered_map<long, int>& walk_to_hop) {
    // search for walks whose destination is the given 'node'.
    auto p1 = equal_range(
        walks.begin(), walks.end(), Encoder::encode(GET_OFFSET(node), 0, 0),
        [&](const long a, const long b) {
            return Encoder::getOffset(a) < Encoder::getOffset(b);
        });
    // Walks satisfying following conditions are selected:
    // 1. strictly < conf_->hos;
    // 2. not expired, i.e., hop < buf_expired_from_[walk_id];
    // 3. if two walks hit a same node, walk with samller hop is selected.
    for (auto it = p1.first; it != p1.second; ++it) {
        if (from_disk && diskExpired(*it)) continue;
        int hop = Encoder::getHop(*it);
        if (hop < conf_->hops) {
            long walk_wo_hop = Encoder::wipeHop(*it);
            auto p2 = walk_to_hop.find(walk_wo_hop);
            if (p2 == walk_to_hop.end() || hop < p2->second)
                walk_to_hop[walk_wo_hop] = hop;
        }
    }
}

vector<long> GreedyManager::getWalksAt(const int node) {
    vector<long> walks_to_resume;
    int bucket = GET_BUCKET(node);

    if (!dsk_walks_.contains(bucket))
        disk_store_.load(bucket, dsk_walks_[bucket]);

    unordered_map<long, int> walk_to_hop;
    getWalkResumingHop(node, dsk_walks_[bucket], true, walk_to_hop);
    getWalkResumingHop(node, buf_walks_[bucket], false, walk_to_hop);

    // prepare the walks to resume
    for (const auto& pr : walk_to_hop)
        walks_to_resume.push_back(Encoder::addHop(pr.first, pr.second));

    return walks_to_resume;
}

void GreedyManager::capture() {
    tmp_walks_ = std::move(ram_store_->bkt_to_walks);
    for (auto& pr : tmp_walks_)
        std::sort(pr.second.begin(), pr.second.end(),
                  std::less<unsigned long>());
    tmp_expired_ = std::move(ram_store_->wid_to_hop);
    tmp_htimes_ = std::move(analyzer_->walk_ht_);
}

void GreedyManager::merge() {
    vector<std::future<size_t>> futures;
    for (int core = 0; core < conf_->n_cpus; core++) {
        int bkt_start = core * conf_->n_bkts_a_cpu,
            bkt_end = std::min(conf_->n_bkts, (core + 1) * conf_->n_bkts_a_cpu);
        futures.push_back(std::async(std::launch::async,
                                     &GreedyManager::mergeToBufferBuckets, this,
                                     bkt_start, bkt_end));
    }
    size_t num_walks = 0;
    for (auto& f : futures) num_walks += f.get();

    // clean temporal walks just for safety (not necessary)
    tmp_walks_.clear();

    // update expired hop information
    for (auto& pr : tmp_expired_)
        if (getDiskExpiredHop(pr.first) > pr.second)
            setDiskExpiredHop(pr.first, pr.second);
    tmp_expired_.clear();

    // update hitting time
    for (auto& pr : tmp_htimes_)
        setByte(conf_->walkIdx(pr.first), pr.second, htimes_);
    tmp_htimes_.clear();

    // flush to disk if buffer is too large
    if (num_walks > 0.6 * conf_->total_walks) mergeToDisk();
}

void GreedyManager::mergeSort(vector<long>& v1, vector<long>& v2,
                              std::function<bool(long)>&& v1_expired,
                              vector<long>& v) {
    size_t SZ1 = v1.size(), SZ2 = v2.size(), i = 0, j = 0;
    v1.push_back(-1);  // NOTE: (uint64)-1 == 0xf...f
    v2.push_back(-1);
    while (true) {
        if (i == SZ1 && j == SZ2) break;
        if ((unsigned long)v1[i] < (unsigned long)v2[j]) {
            if (!v1_expired(v1[i])) v.push_back(v1[i]);
            i++;
        } else {
            v.push_back(v2[j]);
            j++;
        }
    }
}

size_t GreedyManager::mergeToBufferBuckets(const int bkt_start,
                                           const int bkt_end) {
    size_t num_walks = 0;
    // merge walks in temporal into buffer.
    for (int bkt = bkt_start; bkt < bkt_end; bkt++) {
        auto& buf_walks = buf_walks_[bkt];
        vector<long> walks;
        if (tmp_walks_.find(bkt) != tmp_walks_.end()) {
            // merge temp to buf
            auto& tmp_walks = tmp_walks_[bkt];
            walks.reserve(buf_walks.size() + tmp_walks.size());
            mergeSort(buf_walks, tmp_walks,
                      [&](long w) { return bufExpired(w); }, walks);
        } else {
            walks.reserve(buf_walks.size());
            for (long walk : buf_walks)
                if (!bufExpired(walk)) walks.push_back(walk);
        }
        buf_walks = std::move(walks);
        num_walks += buf_walks.size();
    }
    return num_walks;
}

void GreedyManager::mergeToDisk() {
    printf("merging to disk ...\n");
    vector<std::future<size_t>> futures;
    for (int core = 0; core < conf_->n_cpus; core++) {
        int bkt_start = core * conf_->n_bkts_a_cpu;
        int bkt_end = std::min(conf_->n_bkts, (core + 1) * conf_->n_bkts_a_cpu);
        futures.push_back(std::async(std::launch::async,
                                     &GreedyManager::mergeToDiskBuckets, this,
                                     bkt_start, bkt_end));
    }
    size_t num_walks = 0;
    for (auto& f : futures) num_walks += f.get();

    // clean walks in buffer
    for (auto& walks : buf_walks_) walks.clear();

    // update expired hop information
    disk_store_.updateExpiration(expire_);
    std::fill(expire_.begin(), expire_.end(), get4BytesHop(conf_->hops + 1));
}

size_t GreedyManager::mergeToDiskBuckets(const int bkt_start,
                                         const int bkt_end) {
    size_t num_walks = 0;
    // merge walks in temporal into buffer.
    for (int bkt = bkt_start; bkt < bkt_end; bkt++) {
        disk_store_.cleanOldBufferBucket(bkt);
        if (!dsk_walks_.contains(bkt)) {
            disk_store_.saveBufferBucket(bkt, buf_walks_[bkt]);
            continue;
        }
        auto& buf_walks = buf_walks_[bkt];
        auto& disk_walks = dsk_walks_[bkt];
        vector<long> walks;
        walks.reserve(disk_walks.size() + buf_walks.size());
        mergeSort(disk_walks, buf_walks, [&](long w) { return diskExpired(w); },
                  walks);
        disk_walks = std::move(walks);
        num_walks += disk_walks.size();
    }
    return num_walks;
}

double GreedyManager::getAPGain(const int s) const {
    vector<unordered_map<int, double>> nd_delta(2);
    for (auto& pr : analyzer_->walk_ht_) {
        bool hit_pre = isHit(pr.first), hit_now = (pr.second & 0x80) != 0;
        int delta = 0;
        if (!hit_pre && hit_now)
            delta = 1;
        else if (hit_pre && !hit_now)
            delta = -1;
        if (delta != 0) nd_delta[0][Encoder::getSource(pr.first)] += delta;
    }
    for (auto& pr : nd_delta[0]) pr.second /= conf_->walks;

    if (conf_->depth > 0) {
        NodeContext ctx = graph_mgr_->getNodeContext(s);
        double psj = 1.0 / graph_mgr_->getOutDeg(s), pj, pj_new;
        for (int d = 0; d < conf_->depth; d++) {
            auto &r_map = nd_delta[d % 2], &w_map = nd_delta[(d + 1) % 2];
            w_map.clear();
            // i != s
            for (auto& pr : r_map)
                for (int i : graph_mgr_->getInNbrs(pr.first))
                    if (i != s) w_map[i] += graph_mgr_->P(i) * pr.second;
            // i == s
            w_map[s] = graph_mgr_->P(s, -1);
            for (int j : ctx.nbrs_uni_) {
                pj_new = pj = getNodeAP(j);  // get p_j^{T-1}
                if (r_map.find(j) != r_map.end()) pj_new += r_map[j];
                w_map[s] += graph_mgr_->P(s) * pj_new - psj * pj;
            }
        }
    }

    double gain = 0;
    for (auto& pr : nd_delta[conf_->depth % 2]) gain += pr.second;
    return gain / conf_->n_nodes;
}

double GreedyManager::getHTGain(const int s) const {
    // calculate {delta_h_i^{T-D}}_i for each node i.
    vector<unordered_map<int, double>> nd_delta(2);
    for (const auto& pr : analyzer_->walk_ht_) {
        int delta = getHT(pr.first) - (pr.second & 0x7f);
        if (delta != 0) nd_delta[0][Encoder::getSource(pr.first)] += delta;
    }
    for (auto& pr : nd_delta[0]) pr.second /= conf_->walks;
    // refine
    if (conf_->depth > 0) {
        NodeContext ctx = graph_mgr_->getNodeContext(s);
        double psi = 1.0 / graph_mgr_->getOutDeg(s), hi, hi_new;
        for (int d = 0; d < conf_->depth; d++) {
            auto &r_map = nd_delta[d % 2], &w_map = nd_delta[(d + 1) % 2];
            w_map.clear();
            // j != s
            for (auto& pr : r_map) {
                for (int i : graph_mgr_->getInNbrs(pr.first))
                    if (i != s && i != conf_->absb_node)
                        w_map[i] += graph_mgr_->P(i) * pr.second;
            }
            // j = s
            for (int i : ctx.nbrs_uni_) {
                hi_new = hi = getNodeHT(i);
                if (r_map.find(i) != r_map.end()) hi_new -= r_map[i];
                w_map[s] += psi * hi - graph_mgr_->P(s) * hi_new;
            }
        }
    }

    double gain = 0;
    for (auto& pr : nd_delta[conf_->depth % 2]) gain += pr.second;
    return gain / conf_->n_nodes;
}
