#ifndef __LATTICE_H__
#define __LATTICE_H__
#include <cstdint>
#include <vector>
#include "common.h"

// Lattice used to represents the segmentation of sentence piece
class Lattice {
public:
    Lattice();
    virtual ~Lattice();

    struct Node {
        uint32 node_id;         // unique id in the current lattice.
        uint32 pos;             // Unicode position in the sentence
        uint32 length;          // Unicode length
        std::u32string piece;   // sentence piece representation
        int vocab_id;           // vocab id. (maybe -1 for UNK)
        float score;            // logprob of this sentence piece.
        float backtrace_score;  // backtrace info used in viterbi
        Node *prev;             // best previous node on Viterbi path.
    };

    void reset();
    void setSentence(std::u32string sentence);

    // inserts a new node at [pos, pos+length-1].
    // after calling this method, The caller should set Node::score and Node::vocab_id.
    Node *insertNode(int pos, int length);

    // returns viterbi path. All nodes must be populated in advance.
    std::vector<Node *> viterbi();

    // Returns n-best results.
    std::vector<std::vector<Node *>> nbest(size_t nbest_size);

    // Populates marginal probability of every node in this lattice.
    // |freq| is the frequency of the sentence.
    //  for (auto *node : all_nodes_) {
    //    (*expected)[node->id] += marginal_prob_of_node * freq;
    //  }
    // Returns the log-likelihood of this sentence.
    float populateMarginal(float freq, std::vector<float> *expected) const;

private:
    Node *newNode();
    std::u32string __sentence;
    std::vector<std::u32string> __surface;
    std::vector<std::vector<Node *>> __begin_nodes;
    std::vector<std::vector<Node *>> __end_nodes;
    std::vector<Node *> __all_nodes;
};



#endif /* __LATTICE_H__ */
