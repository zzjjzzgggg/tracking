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

#include <iostream>
#include <bitset>

void test_bit_max() {
    int L = 0x11, H = 0x88;

    std::cout << "L: " << std::bitset<8>(L) << std::endl;
    std::cout << "H: " << std::bitset<8>(H) << std::endl;

    int x = 0x14, y = 0x43;
    std::cout << "x: " << std::bitset<8>(x) << std::endl;
    std::cout << "y: " << std::bitset<8>(y) << std::endl;

    int z = ((((x | H) - (y & ~H)) | (x ^ y)) ^ (x | ~y)) & H;
    std::cout << "z: " << std::bitset<8>(z) << std::endl;

    int m = ((((z >> 3) | H) - L) | H) ^ z;
    std::cout << "m: " << std::bitset<8>(m) << std::endl;

    int r = (x & m) | (y & ~m);
    std::cout << "r: " << std::bitset<8>(r) << std::endl;
}

int main(int argc, char* argv[]) {
    test_bit_max();

    return 0;
}
