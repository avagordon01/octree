#include "tree.hh"

#include <random>
#include <iostream>

int main() {
    size_t sidelength = 1024;
    tree::tree<2, size_t, uint32_t, 8> t{sidelength};

    {
        std::random_device rd;
        std::mt19937_64 rng(rd());

        std::uniform_int_distribution<size_t> uniform_dist(0, sidelength - 1);
        uniform_dist(rng);
     
        std::normal_distribution<> normal_dist(4, 2);
        normal_dist(rng);
     
        size_t n = 1000 * 1000 * 10;
        for (size_t i = 0; i < n; i++) {
            std::array<size_t, 2> pos = {
                uniform_dist(rng),
                uniform_dist(rng)
            };
            t.insert_item({}, pos);
        }
        assert(t.items.size() == n);
    }
    return 0;
}
