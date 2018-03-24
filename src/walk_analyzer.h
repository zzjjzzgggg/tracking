#ifndef __WALK_ANALYZER_H__
#define __WALK_ANALYZER_H__

#include "stdafx.h"
#include "config.h"

class WalkAnalyzer {
protected:
    Config* conf_;

public:
    WalkAnalyzer(Config* conf) : conf_(conf) {}

    // the 'walk' resides on 'node', currently
    virtual void analyze(const int node, const long walk) = 0;
};

#endif /* __WALK_ANALYZER_H__ */
