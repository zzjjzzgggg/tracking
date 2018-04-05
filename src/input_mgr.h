/**
 * Copyright (C) by J.Z. (04/05/2018 10:35)
 * Distributed under terms of the MIT license.
 */

#ifndef __DATA_MGR_H__
#define __DATA_MGR_H__

#include "stdafx.h"

/**
 * Abstract class InputMgr:
 *
 * Feed a batch of edges, outputs a set of nodes whose margianl gain changes.
 */
class InputMgr {
public:
    virtual ~InputMgr() {}

    virtual void addEdges(const std::vector<std::pair<int, int>>&) = 0;

    virtual std::vector<int> getAffectedNodes() = 0;

    virtual double getReward(const int) const = 0;

    virtual double getReward(const std::vector<int>&) const = 0;
    virtual double getReward(const std::unordered_set<int>&) const = 0;

    virtual double getGain(const int, const std::vector<int>&) const = 0;
    virtual double getGain(const int, const std::unordered_set<int>&) const = 0;

}; /* InputMgr */

#endif /* __DATA_MGR_H__ */
