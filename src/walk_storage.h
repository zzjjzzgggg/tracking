#ifndef __WALK_STORAGE_H__
#define __WALK_STORAGE_H__

#include "stdafx.h"
#include "config.h"

class WalkStorage {
protected:
    Config* conf_;

public:
    WalkStorage(Config* conf) : conf_(conf) {}

    virtual void store(const int bucket, const long walk) = 0;

    // load walks in bucket, and sort walks by destinations
    virtual void load(const int bucket, vector<long>& walks) {}

    virtual void clear() {}
};

#endif
