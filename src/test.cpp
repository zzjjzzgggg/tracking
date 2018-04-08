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
    ~A() { printf("A deleted\n"); }
};
class B {
private:
    A* a;

public:
    B() { a = new A(); }
    ~B() {
        delete a;
        printf("B deleted\n");
    }
};

void test_delete_in_list() {
    std::list<B*> b_lst;
    B* b = new B();
    b_lst.insert(b_lst.end(), b);
    // for (auto it = b_lst.begin(); it!= b_lst.end(); ++it) delete *it;
    delete b_lst.front();
    b_lst.pop_front();
    // delete b;
}

void test_set_copy() {
    std::unordered_map<int, int> a;
    a[0] = 1;
    std::unordered_map<int, int> b = a;
    b[1] = 2;

    printf("a: %lu, b:%lu\n", a.size(), b.size());
}

int main(int argc, char* argv[]) {
    // test_bit_max();
    // test_delete_in_list();
    test_set_copy();

    return 0;
}
