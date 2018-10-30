#ifndef __COMPACT_TRIE_H__
#define __COMPACT_TRIE_H__

#include <unordered_map>
#include "common.h"

class CompactTrie {
public :
    struct Node {
        Node *parent;
        std::string str;
        std::unordered_map<std::string, Node*> childs;
        bool is_word;
        int32 value;
    };

    CompactTrie();
    ~CompactTrie();
    void build(const char* keys[], int32 values[], size_t num);
    size_t commonPrefixSearch(const std::string &key, std::vector<std::pair<std::string, int32>> &results) const;

    // get all nodes
    size_t getNodeCount() const;
private:
    Node *createNode(bool is_word, const std::string &str="", int32 value=0);
    void insert(Node* node, const std::string &key, int32 value);
    void search(const Node* node, const std::string &prefix, std::vector<std::pair<std::string, int32>> &results) const;
private:
    std::vector<Node *> __all_nodes;
    Node *__root;
};

#endif // !__TRIE_H__
