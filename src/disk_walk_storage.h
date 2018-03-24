#ifndef DISK_WALK_STORAGE_H
#define DISK_WALK_STORAGE_H

#include "stdafx.h"
#include "walk_storage.h"
#include "LRUCache.h"

class DiskWalkStorage : public WalkStorage {
private:
    LRUCache<int, std::unique_ptr<ioutils::IOOut>> cache{128};

protected:
    string getBucketFile(const int bucket) {
        return osutils::join(conf_->walks_dir, "{}.lz"_format(bucket));
    }

public:
    DiskWalkStorage(Config* conf, const bool delete_dir = false)
        : WalkStorage(conf) {
        if (!osutils::exists(conf_->walks_dir))
            osutils::mkdirs(conf_->walks_dir);
        if (delete_dir) {
            osutils::rmfile(conf_->walks_dir);
            osutils::mkdirs(conf_->walks_dir);
        }
    }

    void store(const int bucket, const long walk) override {
        if (!cache.contains(bucket)) {
            string output = getBucketFile(bucket);
            cache.insert(bucket, ioutils::getIOOut(output, true));
        }
        cache[bucket]->save(walk);
    }

    void load(const int bucket, vector<long>& walks) override {
        walks.clear();
        string filename = getBucketFile(bucket);
        if (osutils::exists(filename)) {
            long walk;
            auto in_ptr = ioutils::getIOIn(filename);
            while (!in_ptr->eof()) {
                in_ptr->load(walk);
                walks.push_back(walk);
            }
        } else
            printf("file %s does not exit.\n", filename.c_str());
    }

    void store(const int bucket, const vector<long>& walks) {
        for (long walk : walks) store(bucket, walk);
    }

    void sortBucket(const int bucket) {
        if (cache.contains(bucket)) cache[bucket]->close();
        vector<long> walks;
        load(bucket, walks);
        if (!walks.empty()) {
            std::sort(walks.begin(), walks.end(), std::less<unsigned long>());
            auto out_ptr = ioutils::getIOOut(getBucketFile(bucket));
            for (long walk : walks) out_ptr->save(walk);
        }
    }
};

#endif
