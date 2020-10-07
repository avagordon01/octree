#include "list.hh"

#include <random>
#include <iostream>
#include <algorithm>
#include <chrono>

#include <Eigen/Dense>

int main() {
    using coord = uint64_t;
    list::list<2, coord, uint32_t> l{};

    {
        std::mt19937_64 rng(0xfeed);
        std::normal_distribution<float> normal_dist(0, 1024);
     
        size_t n = 1000 * 1000 * 10;
        std::vector<decltype(l)::item> to_insert;
        for (size_t i = 0; i < n; i++) {
            std::array<coord, 2> pos = {
                static_cast<coord>(normal_dist(rng)),
                static_cast<coord>(normal_dist(rng))
            };
            to_insert.push_back(decltype(l)::item{pos, {}});
        }
        {
            using time_type = std::chrono::time_point<std::chrono::high_resolution_clock>;
            time_type start_time = std::chrono::high_resolution_clock::now();
            l.insert_items(to_insert);
            time_type stop_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = stop_time - start_time;
            std::cout << diff.count() << std::endl;
        }
    }

    {
        std::mt19937_64 rng(0xfeed);
        std::normal_distribution<float> normal_dist(0, 100);

        struct entity {
            Eigen::Vector2f pos;
            Eigen::Vector2f target;
            float max_speed = 4.0;
        };

        std::vector<struct entity> entities (1000 * 1000);

        for (auto& e: entities) {
            e.pos = {
                normal_dist(rng),
                normal_dist(rng),
            };
            e.target = {
                normal_dist(rng),
                normal_dist(rng),
            };
        }

        for (auto& e: entities) {
            Eigen::Vector2f direction = (e.target - e.pos);
            float d = direction.norm();
            if (d > e.max_speed) {
                direction = direction / d * e.max_speed;
            }
            e.pos += direction;

            if (e.pos == e.target) {
                e.target = {
                    normal_dist(rng),
                    normal_dist(rng),
                };
            }
        }

    }

    return 0;
}
