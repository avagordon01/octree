#include <vector>
#include <array>
#include <optional>
#include <unordered_map>
#include <cassert>
#include <cstdint>
#include <cstdbool>
#include <type_traits>
#include <ostream>
#include <iostream>

namespace tree {

template<size_t Dimension, typename InCoord, typename ID, size_t max_items_per_node = 64, bool use_array = false>
struct tree {
private:
    using item_id = uint32_t;
    using node_id = uint32_t;
    using your_id = ID;
    using Coord = typename std::make_unsigned<InCoord>::type;
    using InPosition = std::array<InCoord, Dimension>;
    using Position = std::array<Coord, Dimension>;
    static constexpr size_t num_child_nodes = 1 << Dimension;

    static Position transform_in(InPosition in) {
        if constexpr (std::numeric_limits<InCoord>::is_signed) {
            Position out {};
            for (size_t i = 0; i < Dimension; i++) {
                out[i] = static_cast<Coord>(in[i]) + (static_cast<Coord>(1) << (std::numeric_limits<Coord>::digits - 2));
            }
            return out;
        } else {
            return in;
        }
    }

    struct area {
        Position min; //inclusive
        Position max; //exclusive
        std::ostream& operator<<(std::ostream& os) {
            os << min << " " << max << std::endl;
            return os;
        }
        bool contains(Position p) {
            for (size_t i = 0; i < Dimension; i++)
                if (p[i] < min[i] || p[i] >= max[i])
                    return false;
            return true;
        }
        bool overlaps(area a) {
            for (size_t i = 0; i < Dimension; i++)
                if (a.max[i] < min[i] || a.min[i] > max[i])
                    return false;
            return true;
        }
        area child(size_t child_index) {
            area child = *this;
            Coord half_sidelength = (max[0] - min[0]) / 2;
            for (size_t i = 0; i < Dimension; i++) {
                child.max[i] -= half_sidelength;
            }
            for (size_t i = 0; i < Dimension; i++) {
                bool b = (child_index >> i) & 1;
                child.min[i] += b * half_sidelength;
                child.max[i] += b * half_sidelength;
            }
            return child;
        }
        std::pair<area, node_id> child(Position position) {
            for (size_t i = 0; i < num_child_nodes; i++) {
                area child_node_area = child(i);
                if (child_node_area.contains(position)) {
                    return {child_node_area, i};
                }
            }
            assert(false);
        }
    };

    struct item {
        Position pos;
        your_id id;
        node_id node_index;
    };

    constexpr const static node_id INVALID_NODE_ID = 0;
    struct node {
        node_id parent_node_index = INVALID_NODE_ID;
        node_id child_nodes_index = INVALID_NODE_ID;
        
        typename std::conditional<
            use_array,
            std::array<item_id, max_items_per_node>,
            std::vector<item_id>
        >::type items_indices {};
    };

    constexpr const static size_t root_level = std::numeric_limits<Coord>::digits - 1;
    //FIXME -1 because we can't express an area that covers the whole universe
    area root_area;

    std::vector<node> nodes;
    std::vector<struct item> items;
    std::unordered_map<your_id, item_id> index;

    std::array<std::pair<node_id, area>, root_level + 1> stack;
    size_t stack_valid_depth = 0;
public:

    tree() {
        root_area.min = {};
        root_area.max.fill(Coord{1} << root_level);
        nodes.push_back({});
        stack[0] = {INVALID_NODE_ID, root_area};
    }
    void split_node(node_id id, area a) {
        node& n = nodes[id];
        n.child_nodes_index = nodes.size();
        for (size_t i = 0; i < num_child_nodes; i++) {
            nodes.push_back({});
        }
        assert(id < nodes.size());
        //rebucket items
        for (size_t i = 0; i < n.items_indices.size(); i++) {
            item_id id = n.items_indices[i];
            Position pos = items[id].pos;
            for (size_t j = 0; j < num_child_nodes; j++) {
                area child_node_area = a.child(j);
                if (child_node_area.contains(pos)) {
                    node& child_node = nodes[n.child_nodes_index + j];
                    child_node.items_indices.push_back(id);
                    break;
                }
            }
        }
        nodes[id].items_indices.clear();
    }
    void maybe_merge_child_nodes(node& n) {
        size_t num_items = 0;
        for (size_t i = 0; i < num_child_nodes; i++) {
            num_items += nodes[n.child_node_index + i].items_indices.size();
        }
        if (num_items > max_items_per_node) {
            return;
        }
        //rebucket items
        for (size_t i = 0; i < num_child_nodes; i++) {
            node& c = nodes[n.child_node_index + i];
            n.items_indices.insert(c.items_indices);
            c.~items_indices();
        }
        n.child_node_index = INVALID_NODE_ID;
    }
    std::optional<item_id> find_item(your_id id) {
        if (true) {
            auto search = index.find(id);
            if (search != index.end()) {
                return {search->second};
            } else {
                return std::nullopt;
            }
        } else {
            for (size_t i = 0; i < items.size(); i++) {
                if (items[i].id == id) {
                    return {i};
                }
            }
            return std::nullopt;
        }
    }
    std::pair<node_id, area> find_node(your_id id) {
        find_item(id);
    }
private:
    static size_t msb(unsigned t) {
        if (t != 0) {
            return sizeof(t) * 8 - 1 - __builtin_clz(t);
        } else {
            return -1;
        }
    }
    static size_t msb(unsigned long t) {
        if (t != 0) {
            return sizeof(t) * 8 - 1 - __builtin_clzl(t);
        } else {
            return -1;
        }
    }
    static size_t msb(unsigned long long t) {
        if (t != 0) {
            return sizeof(t) * 8 - 1 - __builtin_clzll(t);
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
public:
    static bool morton_compare(const InPosition &in_a, const InPosition &in_b) {
        Position a = transform_in(in_a);
        Position b = transform_in(in_b);
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
    size_t depth_to_level(size_t depth) {
        assert(depth <= root_level);
        return root_level - depth;
    }
    size_t level_to_depth(size_t level) {
        assert(level <= root_level);
        return root_level - level;
    }
private:
    void check_stack() {
        size_t check_depth;
        for (check_depth = 0; check_depth <= stack_valid_depth; check_depth++) {
            auto area = stack[check_depth].second;
            Coord size = area.max[0] - area.min[0];
            for (size_t i = 1; i < Dimension; i++) {
                assert(area.max[i] - area.min[i] == size);
            }
            assert(size == (static_cast<Coord>(1) << depth_to_level(check_depth)));
        }
    }
    size_t get_ancestor_depth(Position position) {
        auto [_, ancestor_area] = stack[stack_valid_depth];
        size_t b = highest_bit_different(ancestor_area.min, position);
        size_t level = b != -1ULL ? b + 1 : 0;
        size_t depth = level_to_depth(level);
        size_t valid_depth = std::min(depth, stack_valid_depth);
        if (false) {
            {
                size_t expected_size = static_cast<Coord>(1) << depth_to_level(valid_depth);
                auto area = stack[valid_depth].second;
                size_t got_size = area.max[0] - area.min[0];
                assert(got_size == expected_size);
            }
            check_stack();
        }
        assert(stack[valid_depth].second.contains(position));
        return valid_depth;
    }
public:
    std::tuple<node_id, area, size_t> find_node(Position position) {
        size_t cur_node_id;
        area cur_node_area;
        size_t depth;
        if (true) {
            depth = get_ancestor_depth(position);
            std::tie(cur_node_id, cur_node_area) = stack[depth];
            assert(cur_node_area.contains(position));
        } else {
            cur_node_id = 0;
            cur_node_area = root_area;
            depth = 0;
        }
        while (true) {
            node& n = nodes[cur_node_id];
            if (n.child_nodes_index == INVALID_NODE_ID) {
                //found leaf
                return {cur_node_id, cur_node_area, depth_to_level(depth)};
            } else {
                //traverse
                auto [child_area, child_offset] = cur_node_area.child(position);
                cur_node_id = n.child_nodes_index + child_offset;
                cur_node_area = child_area;
                depth++;
                stack[depth] = {cur_node_id, cur_node_area};
                stack_valid_depth = depth;
            }
        }
    }
    void reserve(size_t n) {
        nodes.reserve(n);
        items.reserve(n);
    }
    item_id insert_item(your_id data, InPosition in_position) {
        Position position = transform_in(in_position);
        if (!root_area.contains(position)) {
            std::cerr << "warning: cannot insert_item " << data << " with position out of bounds" << std::endl;
            return -1;
        }
        item_id id = items.size();
        items.push_back({position, data, id});
        auto [node_id, node_area, node_level] = find_node(position);
        node& n = nodes[node_id];
        //found leaf
        if (n.items_indices.size() < max_items_per_node || node_level == 0) {
            //insert
            n.items_indices.push_back(id);
        } else {
            split_node(node_id, node_area);
            for (size_t j = 0; j < num_child_nodes; j++) {
                area child_node_area = node_area.child(j);
                if (child_node_area.contains(position)) {
                    node& child_node = nodes[n.child_nodes_index + j];
                    child_node.items_indices.push_back(id);
                    break;
                }
            }
        }
        return id;
    }

    void remove_item(item_id id);
    node& get_node(item_id id);
    std::vector<item&> get_items(node n);
    std::vector<item_id> get_items_within_area(area a);

    void check_items_in_tree() {
        size_t items_in_tree = 0;
        for (node& n: nodes) {
            items_in_tree += n.items_indices.size();
        }
        assert(items_in_tree == items.size());
    }
};

}
