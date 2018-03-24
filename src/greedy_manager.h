#ifndef __GREEDY_MANAGER_H__
#define __GREEDY_MANAGER_H__

#include "stdafx.h"
#include "config.h"
#include "ram_walk_storage.h"
#include "disk_greedy_storage.h"
#include "hitting_time_analyzer.h"
#include "LRUCache.h"
#include "graph_manager.h"

class GreedyManager {
private:
    Config* conf_;
    RAMWalkStorage* ram_store_;
    HittingTimeAnalyzer* analyzer_;
    GraphManager* graph_mgr_;

    DiskGreedyStorage disk_store_;

    // bucket -> walks
    LRUCache<int, vector<long>> dsk_walks_{1024};

    // "buf_walks_": always contains the latest walks.
    // "expire_": specifies the hop from which the walks in 'dsk_walks_' are
    // expired.
    // The highest bit of 'htimes_' indicatas hit or miss.
    vector<vector<long>> buf_walks_;
    vector<int> htimes_, expire_;

    // temporal walks captured from RAM storage
    // "tmp_expired_": specifies the hop from which the walks in buffer and disk
    // are expired.
    unordered_map<int, vector<long>> tmp_walks_;
    unordered_map<long, int> tmp_expired_, tmp_htimes_;

private:
    inline int getHT(const long walk) const {
        return getByte(conf_->walkIdx(walk), htimes_) & 0x7f;
    }
    inline bool isHit(const long walk) const {
        return (getByte(conf_->walkIdx(walk), htimes_) & 0x80) != 0;
    }

    inline int getDiskExpiredHop(const long walk) const {
        return getByte(conf_->walkIdx(walk), expire_);
    }
    inline void setDiskExpiredHop(const long walk, const int hop) {
        setByte(conf_->walkIdx(walk), hop, expire_);
    }

    inline bool diskExpired(const long walk) const {
        return Encoder::getHop(walk) >= getDiskExpiredHop(walk);
    }
    bool bufExpired(const long walk) const {
        auto it = tmp_expired_.find(Encoder::getWalkId(walk));
        return it != tmp_expired_.end() && Encoder::getHop(walk) >= it->second;
    }

    double getNodeHT(const int source) const {
        double sum_ht = 0;
        for (int id = 0; id < conf_->walks; id++)
            sum_ht += getHT(Encoder::encode(0, source, id, 0));
        return sum_ht / conf_->walks;
    }
    double getNodeAP(const int source) const {
        double sum = 0;
        for (int id = 0; id < conf_->walks; id++)
            sum += isHit(Encoder::encode(0, source, id, 0));
        return sum / conf_->walks;
    }

    void getWalkResumingHop(const int node, const vector<long>& walks,
                            const bool from_disk,
                            unordered_map<long, int>& walk_to_hop);

    size_t mergeToBufferBuckets(const int bucket_start, const int bucket_end);
    size_t mergeToDiskBuckets(const int bucket_start, const int bucket_end);
    void mergeToDisk();
    void mergeSort(vector<long>& v1, vector<long>& v2,
                   std::function<bool(long)>&& v1_expired, vector<long>& v);

    double getAPGain(const int s) const;
    double getHTGain(const int s) const;

public:
    GreedyManager(Config* conf, HittingTimeAnalyzer* analyzer,
                  GraphManager* graph_mgr, RAMWalkStorage* ram_store = nullptr)
        : conf_(conf), disk_store_(conf), analyzer_(analyzer),
          graph_mgr_(graph_mgr), ram_store_(ram_store) {
        htimes_.resize(conf_->total_walks / 4 + 1, get4BytesHop(conf_->hops));
        expire_.resize(conf_->total_walks / 4 + 1,
                       get4BytesHop(conf_->hops + 1));
        buf_walks_.resize(conf_->n_bkts);
    }

    // /**
    //  * This function is used in the oracle call experiment.
    //  */
    // void reset() {
    //     std::fill(htimes_.begin(), htimes_.end(), get4BytesHop(conf_->hops));
    //     std::fill(expire_.begin(), expire_.end(),
    //               get4BytesHop(conf_->hops) + 1);
    //     buf_walks_.clear();
    //     dsk_walks_.clear();
    // }

    /**
     * Get the sources of walks that hit the given 'node'.
     * Walks from these sources need to resume.
     */
    vector<long> getWalksAt(const int node);

    double getGain(const int s) {
#ifdef GAIN_AP
        return getAPGain(s);
#else
        return getHTGain(s);
#endif
    }

    /**
     * save current new walks in 'ram_store_' to 'tmp_walks_'.
     */
    void capture();

    /**
     * Merge temporal walks into buffered walks:
     *  1. delete expired walks in 'buf_walks_';
     *  2. add new walks in 'tmp_walks_' to 'buf_walks_';
     *  3. update 'buf_walkid_hop_vec_';
     *  4. update 'htimes_'.
     *
     * Here we assume that the affected walks can be stored in RAM because we
     * usually select a small fraction of nodes. If this is not the case, we
     * need to flush walks from RAM to disk when necessary.
     */
    void merge();
};
#endif /* __GREEDY_MANAGER_H__ */
