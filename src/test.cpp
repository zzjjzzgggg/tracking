#include <cstdio>
#include <cassert>
#include <cstring>
#include <cmath>

#include <limits>
#include <iomanip>
#include <iostream>
#include <vector>
#include <queue>
#include <list>
#include <unordered_map>
#include <unordered_set>

#include <iostream>
#include <bitset>

#include <adv/LRUCache.h>

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

class A {
public:
    A() {}
    ~A() {}
};

class B {
public:
    int l;
    A* a;

public:
    B(int x) : l(x) { a = new A(); }
    ~B() { delete a; }
};

void test_copy_in_list() {
    std::list<B*> lst;
    B* b1 = new B(1);
    B* b2 = new B(*b1);

    lst.insert(lst.end(), b1);
    lst.insert(lst.end(), b2);

    b2->l++;

    for (auto it = lst.begin(); it != lst.end(); ++it) printf("%d\n", (*it)->l);
}

void test_lru() {
    lru::Cache<std::string, std::string> cache(3);
    cache.insert("hello", "world");
    cache.insert("foo", "bar");

    std::cout << "checking refresh : " << cache.get("hello") << std::endl;
    cache.insert("hello1", "world1");
    cache.insert("foo1", "bar1");
}

void test_hash() {
    int x = 184615, y = 364376;
    size_t val1 = (std::hash<int>()(x) << 32) | std::hash<int>()(y);
    size_t val2 = (std::hash<int>()(y) << 32) | std::hash<int>()(x);

    std::cout << val1 << std::endl;
    std::cout << val2 << std::endl;
}

int main(int argc, char* argv[]) {
    // test_bit_max();
    // test_copy_in_list();
    // test_lru();
    test_hash();

    return 0;
}
