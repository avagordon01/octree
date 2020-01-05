#include "tree.hh"

#include <random>
#include <iostream>
#include <algorithm>
#include <chrono>

int main() {
    tree::tree<2, size_t, uint32_t> t{};

    assert(t.msb(0ULL) == -1ULL);
    assert(t.msb(1ULL) == 0);
    assert(t.msb(2ULL) == 1);

    {
        std::mt19937_64 rng(0xfeed);

        size_t sidelength = 1024;
        std::uniform_int_distribution<size_t> uniform_dist(0, sidelength - 1);
        uniform_dist(rng);
     
        std::normal_distribution<> normal_dist(4, 2);
        normal_dist(rng);
     
        size_t n = 1000 * 1000 * 10;
        std::vector<decltype(t)::Position> to_insert;
        for (size_t i = 0; i < n; i++) {
            std::array<size_t, 2> pos = {
                uniform_dist(rng),
                uniform_dist(rng)
            };
            to_insert.push_back(pos);
        }
        bool morton_sort = false;
        if (morton_sort) {
            std::sort(to_insert.begin(), to_insert.end(), decltype(t)::morton_compare);
        }
        t.nodes.reserve(n);
        t.items.reserve(n);
        {
            using time_type = std::chrono::time_point<std::chrono::high_resolution_clock>;
            time_type start_time = std::chrono::high_resolution_clock::now();
            for (auto &pos: to_insert) {
                t.insert_item({}, pos);
            }
            time_type stop_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = stop_time - start_time;
            std::cout << diff.count() << std::endl;
        }
        assert(t.items.size() == n);
    }
    return 0;
}
