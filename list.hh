#include <vector>
#include <optional>
#include <algorithm>

namespace list {

template<size_t Dimension, typename Coord, typename ID>
struct list {
    using item_id = size_t;
    using your_id = ID;
    using Position = std::array<Coord, Dimension>;

    struct item {
        Position pos;
        your_id id;

        item() {}
        item(Position p, your_id d):
            pos(p),
            id(d)
        {}
        item(const item& other) = delete;
        item(item&& other) {
            id = other.id;
            pos = other.pos;
        }
        item& operator=(const item& other) = delete;
        item& operator=(item&& other) {
            id = other.id;
            pos = other.pos;
            return *this;
        }
    };

    std::vector<item> items;
    std::unordered_map<your_id, item_id> index;

    template<typename T>
    static size_t msb(T t) {
        if (t != 0) {
            if constexpr (sizeof(t) == 4) {
                return std::numeric_limits<T>::digits - 1 - __builtin_clzl(t);
            } else if (sizeof(t) == 8) {
                return std::numeric_limits<T>::digits - 1 - __builtin_clzll(t);
            }
        } else {
            return -1;
        }
    }
    static size_t highest_bit_different(Position a, Position b) {
        Coord bits = 0;
        for (size_t i = 0; i < Dimension; i++)
            bits |= a[i] ^ b[i];
        return msb(bits);
    }
    static bool morton_compare(const Position &a, const Position &b) {
        size_t bit = highest_bit_different(a, b);
        size_t bits_a = 0;
        size_t bits_b = 0;
        for (size_t i = 0; i < Dimension; i++) {
            bits_a <<= 1;
            bits_b <<= 1;
            bits_a |= (a[i] >> bit) & 1;
            bits_b |= (b[i] >> bit) & 1;
        }
        return bits_a < bits_b;
    }

    void insert_items(std::vector<item>& is) {
        auto comp = [](const auto& a, const auto& b) -> bool {
            return morton_compare(a.pos, b.pos);
        };
        std::sort(is.begin(), is.end(), comp);
        auto middle = items.end();
        items.insert(
            items.end(),
            std::make_move_iterator(is.begin()),
            std::make_move_iterator(is.end())
        );
        std::inplace_merge(items.begin(), middle, items.end(), comp);
    }
    void insert_item(your_id data, Position position) {
        items.push_back(std::move(item{position, data}));
        std::sort(items.begin(), items.end(),
            [](const auto& a, const auto& b) -> bool {
                return morton_compare(a.pos, b.pos);
            }
        );
    }

    std::optional<Position> find_item(your_id data) {
        auto search = index.find(data);
        if (search != index.end()) {
            return items[search->second].pos;
        } else {
            return std::nullopt;
        }
    }

    std::optional<your_id> find_item(Position position) {
        if (!items.empty()) {
            auto search = std::lower_bound(items.begin(), items.end(),
                [](const auto& a, const auto& b) -> bool {
                    return morton_compare(a.pos, b.pos);
                }
            );
            if (search->pos == position) {
                return {search->id};
            }
        }
        return std::nullopt;
    }
};

}
