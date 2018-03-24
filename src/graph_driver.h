#ifndef __GRAPH_DRIVER_H__
#define __GRAPH_DRIVER_H__

#include "stdafx.h"
#include "config.h"

/**
 * GraphDriver is responsible for efficiently providing necessary graph data to
 * WalkEngine.
 */
class GraphDriver {
protected:
    Config* conf_;

public:
    GraphDriver(Config* conf) : conf_(conf) {}

    virtual void reset() {}

    virtual NodeContext getNodeContext(const int node) = 0;
};

#endif
