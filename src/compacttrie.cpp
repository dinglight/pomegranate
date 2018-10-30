#include "compacttrie.h"
#include <algorithm>
#include "util.h"


std::string get_common_prefix(const std::string &str1, const std::string str2)
{
    std::string common_prefix = "";
    size_t min_len = std::min(str1.length(), str2.length());
    for (size_t i = 0; i < min_len; ++i) {
        if (str1.at(i) == str2.at(i)) {
            common_prefix += str1.at(i);
        } else {
            break;
        }
    }
    return common_prefix;
}

CompactTrie::CompactTrie()
    :__root(nullptr)
{
    __root = createNode(false);
}

CompactTrie::~CompactTrie()
{
    __root = nullptr;

    util::stl::delete_vector_elements(&__all_nodes);
}

void CompactTrie::build(const char* keys[], int32 values[], size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        std::string key = keys[i];
        int32 value = values[i];
        insert(__root, key, value);
    }
}

size_t CompactTrie::commonPrefixSearch(const std::string &key, std::vector<std::pair<std::string, int32>> &results) const
{
    Node *node = __root;
    std::string key_left = key;
    for (const auto &p : node->childs) {
        std::string common_prefix = get_common_prefix(key_left, p.first);
        if (common_prefix.length() > 0 && common_prefix.length() == key_left.length()) {
            // key is a prefix of node str
            // collect all sub node
            search(p.second, common_prefix, results);
            break;
        } else if (common_prefix.length() > 0 && common_prefix.length() < key_left.length()) {
            node = p.second;
            key_left = key_left.substr(common_prefix.length());
        }
    }
    return results.size();
}

size_t CompactTrie::getNodeCount() const
{
    return __all_nodes.size();
}
////////////////////////////////////////////////////////////////////////////

CompactTrie::Node *CompactTrie::createNode(bool is_word, const std::string &str, int32 value)
{
    Node *node = new Node();
    node->parent = nullptr;
    node->is_word = is_word;
    node->str = str;
    node->value = value;
    __all_nodes.push_back(node);
    return node;
}

void CompactTrie::insert(Node *node, const std::string &key, int32 value)
{
    std::string str = node->str;
    // common prefix
    std::string common_prefix = get_common_prefix(key, str);


    // found common prefix
    if (common_prefix.length() > 0 && common_prefix.length() < str.length()) {
        Node *split_node = createNode(false, common_prefix, value);
        split_node->parent = node->parent;
        split_node->parent->childs.erase(node->str);
        split_node->parent->childs[common_prefix] = split_node;
        std::string str_left = str.substr(common_prefix.length());
        node->str = str_left;
        split_node->childs[str_left] = node;
        node->parent = split_node;
        std::string key_left = key.substr(common_prefix.length());
        Node *new_node = createNode(true, key_left, value);
        split_node->childs[key_left] = new_node;
        new_node->parent = split_node;
    } else {
        std::string key_left = key.substr(common_prefix.length());
        bool is_find = false;
        for (const auto &p : node->childs) {
            Node * child_node = p.second;
            if (key_left.at(0) == child_node->str.at(0)) {
                is_find = true;
                insert(child_node, key_left, value);
                break;
            }
        }
        if (!is_find) {
            Node *new_node = createNode(true, key_left, value);
            node->childs[key_left] = new_node;
            new_node->parent = node;
        }
    }
}

void CompactTrie::search(const Node* node, const std::string &prefix, std::vector<std::pair<std::string, int32>> &results) const
{
    if (node == nullptr)
        return;

    if (node->is_word) {
        results.push_back(std::make_pair(prefix, node->value));
    }

    for (const auto &p : node->childs) {
        search(p.second, prefix + p.first, results);
    }
}