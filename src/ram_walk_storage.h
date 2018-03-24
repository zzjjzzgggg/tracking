#ifndef __RAM_WALK_STORAGE_H__
#define __RAM_WALK_STORAGE_H__

#include "stdafx.h"
#include "walk_storage.h"

class RAMWalkStorage : public WalkStorage {
public:
    unordered_map<int, vector<long>> bkt_to_walks;

    // walk_id -> minimum hop where the walk is renewed.
    unordered_map<long, int> wid_to_hop;

public:
    RAMWalkStorage(Config* conf) : WalkStorage(conf) {}

    void store(const int bucket, const long walk) override {
        bkt_to_walks[bucket].push_back(walk);
        long wid = Encoder::getWalkId(walk);
        int hop = Encoder::getHop(walk);
        if (wid_to_hop.find(wid) == wid_to_hop.end() || hop < wid_to_hop[wid])
            wid_to_hop[wid] = hop;
    }

    void clear() override {
        bkt_to_walks.clear();
        wid_to_hop.clear();
    }
};

#endif /* __RAM_WALK_STORAGE_H__ */
