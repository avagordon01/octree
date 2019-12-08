#include <vector>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdbool>
#include <type_traits>
#include <ostream>

namespace tree {

template<size_t Dimension, typename Coord, typename ID, size_t max_items_per_node = 64, bool use_array = false>
struct tree {
    using item_id = uint32_t;
    using node_id = uint32_t;
    using your_id = ID;
    using Position = std::array<Coord, Dimension>;
    static constexpr size_t num_child_nodes = 1 << Dimension;

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
        };
        bool overlaps(area a) {
            for (size_t i = 0; i < Dimension; i++)
                if (a.max[i] < min[i] || a.min[i] > max[i])
                    return false;
            return true;
        };
        area child(size_t child_index) {
            area child = *this;
            Coord half_sidelength = (max[0] - min[0]) / 2;
            for (size_t i = 0; i < Dimension; i++) {
                child.max[i] -= half_sidelength;
            }
            for (size_t i = 0; i < Dimension; i++) {
                if (((child_index >> i) & 1) == 1) {
                    child.min[i] += half_sidelength;
                    child.max[i] += half_sidelength;
                }
            }
            return child;
        };
    };

    struct item {
        Position pos;
        your_id id;
        node_id node_index;
    };

    struct node {
        node_id parent_node_index;
        node_id child_nodes_index;
        
        typename std::conditional<
            use_array,
            std::array<item_id, max_items_per_node>,
            std::vector<item_id>
        >::type items_indices;
    };

    const node_id INVALID_NODE_ID = 0;

    const Coord root_sidelength;
    area root_area;

    std::vector<node> nodes;
    std::vector<struct item> items;

    tree(Coord root_sidelength_) :
        root_sidelength(root_sidelength_)
    {
        root_area.min = {};
        root_area.max.fill(root_sidelength_);
        nodes.push_back({INVALID_NODE_ID, INVALID_NODE_ID, {}});
    }
    void split_node(node_id id, area a) {
        node n = nodes[id];
        n.child_nodes_index = nodes.size();
        for (size_t i = 0; i < num_child_nodes; i++) {
            nodes.push_back({INVALID_NODE_ID, INVALID_NODE_ID, {}});
        }
        assert(id < nodes.size());
        //rebucket items
        for (size_t i = 0; i < n.items_indices.size(); i++) {
            item_id id = n.items_indices[i];
            Position pos = items[id].pos;
            size_t j;
            for (j = 0; j < num_child_nodes; j++) {
                area child_node_area = a.child(j);
                if (child_node_area.contains(pos)) {
                    node& child_node = nodes[n.child_nodes_index + j];
                    child_node.items_indices.push_back(id);
                    break;
                }
            }
            assert(j < num_child_nodes);
        }
        nodes[id].items_indices.clear();
    };
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
    };
    std::pair<node_id, area> find_node(Position position) {
        node_id cur_node_id = 0;
        area cur_node_area = root_area;
        assert(cur_node_area.contains(position));
        for (size_t level = 0; ; level++) {
            node& n = nodes[cur_node_id];
            if (n.child_nodes_index == INVALID_NODE_ID) {
                //found leaf
                return {cur_node_id, cur_node_area};
            } else {
                //traverse
                size_t i;
                for (i = 0; i < num_child_nodes; i++) {
                    area child_node_area = cur_node_area.child(i);
                    if (child_node_area.contains(position)) {
                        node_id child_node_id = n.child_nodes_index + i;
                        cur_node_id = child_node_id;
                        cur_node_area = child_node_area;
                        break;
                    }
                }
                assert(i < num_child_nodes);
            }
        }
    };
    item_id insert_item(your_id data, Position position) {
        item_id id = items.size();
        items.push_back({position, data, id});
        auto p = find_node(position);
        node& n = nodes[p.first];
        //found leaf
        if (n.items_indices.size() >= max_items_per_node) {
            split_node(p.first, p.second);
        } else {
            //insert
            n.items_indices.push_back(id);
        }
        return id;
    };

    void remove_item(item_id id);
    node& get_node(item_id id);
    std::vector<item&> get_items(node n);
    std::vector<item_id> get_items_within_area(area a);
};

}
