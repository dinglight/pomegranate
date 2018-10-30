#ifndef __TRIE_H__
#define __TRIE_H__

#include <unordered_map>
#include "common.h"

class Trie {
public :
    struct Node {
        std::unordered_map<char, Node*> childs;
        bool leaf;      // end of word
    };

    Trie();
    ~Trie();
    void build(const char* keys[], int32 values[], size_t num);
    size_t commonPrefixSearch(const std::string &key, std::vector<std::pair<std::string, int32>> &results) const;

    // get all nodes
    size_t getNodeCount() const;
private:
    Node *newNode();
    void insert(const std::string &key, int32 value);
    void search(const Node* node, const std::string &prefix, std::vector<std::pair<std::string, int32>> &results) const;
private:
    std::vector<Node *> __all_nodes;
    Node *__root;
};

#endif // !__TRIE_H__
