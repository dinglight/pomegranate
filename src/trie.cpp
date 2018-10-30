#include "trie.h"
#include "util.h"
Trie::Trie()
    :__root(nullptr)
{

}

Trie::~Trie()
{
    __root = nullptr;

    util::stl::delete_vector_elements(&__all_nodes);
}

void Trie::build(const char* keys[], int32 values[], size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        std::string key = keys[i];
        int32 value = values[i];
        insert(key, value);
    }
}

size_t Trie::commonPrefixSearch(const std::string &key, std::vector<std::pair<std::string, int32>> &results) const
{
    Node *p = __root;
    std::string prefix = "";
    for (const char &c : key) {
        if (p->childs.find(c) == p->childs.end()) {
            return 0;
        }
        p = p->childs[c];
        prefix.push_back(c);
    }

    // all words with the prefix
    search(p, prefix, results);
    return results.size();
}

size_t Trie::getNodeCount() const
{
    return __all_nodes.size();
}
////////////////////////////////////////////////////////////////////////////

Trie::Node *Trie::newNode()
{
    Node *node = new Node();
    node->leaf = false;
    __all_nodes.push_back(node);
    return node;
}

void Trie::insert(const std::string &key, int32 value)
{
    if (__root == nullptr) {
        __root = newNode();
    }

    Node *node = __root;
    for (const char &c : key) {
        if (node->childs.find(c) == node->childs.end()) {
            node->childs[c] = newNode();
        }
        node = node->childs[c];
    }
    node->leaf = true;
}

void Trie::search(const Node* node, const std::string &prefix, std::vector<std::pair<std::string, int32>> &results) const
{
    if (node == nullptr) {
        return;
    }

    if (node->leaf) {
        results.push_back(std::make_pair(prefix, 1));
    }

    for (const auto &it : node->childs) {
        search(it.second, prefix + it.first, results);
    }
}