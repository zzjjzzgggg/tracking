#ifndef __DISK_GREEDY_STORAGE_H__
#define __DISK_GREEDY_STORAGE_H__

#include "stdafx.h"
#include "disk_walk_storage.h"

class DiskGreedyStorage : public DiskWalkStorage {
private:
    string greedy_dir_;
    vector<int> expired_from_;

private:
    string getBufferBucketFile(const int bucket) {
        return osutils::join(greedy_dir_, "{}.lz"_format(bucket));
    }

    bool expired(const long walk) {
        return Encoder::getHop(walk) >=
               getByte(conf_->walkIdx(walk), expired_from_);
    }

public:
    DiskGreedyStorage(Config* conf, const bool rmdir = true)
        : DiskWalkStorage(conf) {
        greedy_dir_ = osutils::join(conf_->walks_dir, "greedy");

        if (!osutils::exists(greedy_dir_)) osutils::mkdirs(greedy_dir_);
        if (rmdir) {
            osutils::rmfile(greedy_dir_);
            osutils::mkdirs(greedy_dir_);
        }

        expired_from_.resize(conf_->total_walks / 4 + 1,
                             get4BytesHop(conf_->hops + 1));
    }

    void saveBufferBucket(const int bucket, const vector<long>& walks) {
        auto out_ptr = ioutils::getIOOut(getBufferBucketFile(bucket));
        for (long walk : walks) out_ptr->save(walk);
    }

    void cleanOldBufferBucket(const int bucket) {
        osutils::rmfile(getBufferBucketFile(bucket));
    }

    /**
     * Load unexpired disk walks into bucket.
     * If buffered walks exist, also read them.
     */
    void load(const int bucket, vector<long>& walks) override {
        walks.clear();
        string disk_bkt_fnm = getBucketFile(bucket);
        string buf_bkt_fnm = getBufferBucketFile(bucket);
        auto disk_ptr = osutils::exists(disk_bkt_fnm)
                            ? ioutils::getIOIn(disk_bkt_fnm)
                            : nullptr;
        auto buf_ptr = osutils::exists(buf_bkt_fnm)
                           ? ioutils::getIOIn(buf_bkt_fnm)
                           : nullptr;
        long disk_walk, buf_walk;
        bool read_disk = true, read_buf = true;
        while (true) {
            if (read_disk && disk_ptr != nullptr && !disk_ptr->eof())
                disk_ptr->load(disk_walk);
            else if (read_disk)
                disk_walk = -1;
            if (read_buf && buf_ptr != nullptr && !buf_ptr->eof())
                buf_ptr->load(buf_walk);
            else if (read_buf)
                buf_walk = -1;
            if (disk_walk == -1 && buf_walk == -1) break;
            if ((unsigned long)disk_walk < (unsigned long)buf_walk) {
                if (!expired(disk_walk)) walks.push_back(disk_walk);
                read_disk = true;
                read_buf = false;
            } else {
                walks.push_back(buf_walk);
                read_disk = false;
                read_buf = true;
            }
        }
    }

    /**
     * Update expiration information that describes the expired
     * hop of each walk.
     */
    void updateExpiration(const vector<int>& expired_hop) {
        for (size_t i = 0; i < expired_hop.size(); i++)
            if (expired_hop[i] < expired_from_[i])
                expired_from_[i] = expired_hop[i];
    }
};

#endif
