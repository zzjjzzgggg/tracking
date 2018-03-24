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

int main(int argc, char* argv[]) {
    std::unordered_map<int, int> m;
    m[1] = m.find(1) == m.end() ? 0 : 1;
    printf("%d\n", m[1]);

    return 0;
}
