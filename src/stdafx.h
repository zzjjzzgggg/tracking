#pragma once

#include <cstdio>
#include <cassert>
#include <cstring>
#include <cmath>

#include <limits>
#include <iomanip>
#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#include <mutex>
#include <future>
#include <thread>

#include <os/osutils.h>
#include <io/ioutils.h>
#include <adv/rngutils.h>
#include <graph/graph.h>

#define BUCKET_SIZE 256
#define MAX_HOP 256

#define GET_BUCKET(node) ((node) / BUCKET_SIZE)
#define GET_OFFSET(node) ((node) % BUCKET_SIZE)
#define FIRST_NODE_IN_BUCKET(bucket) ((bucket)*BUCKET_SIZE)

using std::vector;
using std::string;
using std::unordered_map;
using std::unordered_set;

using namespace graph;
using namespace fmt::literals;
