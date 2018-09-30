#include "Lattice.h"
#include <algorithm>
#include <queue>
#include "log.h"
#include "util.h"

// Returns log(exp(x) + exp(y)).
// if init_mode is true, returns log(exp(y)) == y.
// log(\sum_i exp(a[i])) can be computed as
// for (int i = 0; i < a.size(); ++i)
//   x = LogSumExp(x, a[i], i == 0);
static float log_sum_exp(float x, float y, bool init_mode)
{
    if (init_mode) {
        return y;
    }

    const float vmin = std::min(x, y);
    const float vmax = std::max(x, y);
    constexpr float kMinusLogEpsilon = 50;
    if (vmax > vmin + kMinusLogEpsilon) {
        return vmax;
    } else {
        return vmax + log(exp(vmin - vmax) + 1.0);
    }
}

Lattice::Lattice()
{
}

Lattice::~Lattice()
{
    reset();
}
void Lattice::reset()
{
    __begin_nodes.clear();
    __end_nodes.clear();
    __sentence.clear();
    util::stl::delete_vector_elements(&__all_nodes);
}
void Lattice::setSentence(std::u32string sentence)
{
    reset();
    __sentence = sentence;
    for (size_t offset = 0; offset < __sentence.length(); ++offset) {
        __surface.push_back(__sentence.substr(offset));
    }

    const size_t len = __surface.size();
    __begin_nodes.resize(len + 1);
    __end_nodes.resize(len + 1);

    for (size_t i = 0; i < len; ++i) {
        __begin_nodes[i].reserve(16);
        __end_nodes[i].reserve(16);
    }

    Node *bos = newNode();
    bos->vocab_id = -1;
    bos->pos = 0;
    __end_nodes[0].push_back(bos);

    Node *eos = newNode();
    eos->vocab_id = -1;
    eos->pos = len;
    __begin_nodes[len].push_back(eos);
}

Lattice::Node *Lattice::insertNode(int pos, int length)
{
    Node *node = newNode();
    node->pos = pos;
    node->length = length;
    node->piece = __surface[pos].substr(0, length);
    __begin_nodes[pos].push_back(node);
    __end_nodes[pos + node->length].push_back(node);

    return node;
}

std::vector<Lattice::Node *> Lattice::viterbi()
{
    const size_t len = __sentence.length();
    for (size_t pos = 0; pos <= len; ++pos) {
        for (Node *rnode : __begin_nodes[pos]) {
            rnode->prev = nullptr;
            float best_score = 0.0;
            Node *best_node = nullptr;
            for (Node *lnode : __end_nodes[pos]) {
                const float score = lnode->backtrace_score + rnode->score;
                if (best_node == nullptr || score > best_score) {
                    best_node = lnode;
                    best_score = score;
                }
            }
            if (best_node == nullptr) {
                return {};
            }
            rnode->prev = best_node;
            rnode->backtrace_score = best_score;
        }
    }

    // backtrace
    std::vector<Node *> results;
    for (Node *node = __begin_nodes[len][0]->prev; node->prev != nullptr;
         node = node->prev) {
        results.push_back(node);
    }
    std::reverse(results.begin(), results.end());
    return results;
}

// search to enumerate the nbests by A* algorithm
// Given a lattice, enumerates Cells (paths) from EOS.
// At each partial path x,compute f(x) as follows
//   f(x) = g(x) + h(x).
// g(x): the sum of scores from  EOS to the left-most node in x.
// h(x): a heuristic that estimates the largest score from x to BOS.
// f(x): the priority to pop a new hypothesis from the priority queue.
//
// As left-to-right Viterbi search can tell the *exact* value of h(x),
// we can obtain the exact n-best results with A*.
// 
struct Cell {
    Lattice::Node *node;
    Cell *next;
    float fx;
    float gx;
};

class CellComparator {
public:
    const bool operator()(Cell *c1, Cell *c2)
    {
        return (c1->fx < c2->fx);
    }
};

Cell *new_cell(std::vector<Cell *> &allocated)
{
    Cell *cell = new Cell;
    memset(cell, 0, sizeof(*cell));
    allocated.push_back(cell);
    return cell;
}
std::vector<std::vector<Lattice::Node *>> Lattice::nbest(size_t nbest_size)
{
    if (nbest_size < 1) {
        LOG(W, "nbest_size < 1, return empty result.");
        return {};
    }

    if (nbest_size == 1) {
        return{ viterbi() };
    }

    std::priority_queue<Cell *, std::vector<Cell *>, CellComparator> open_list;
    std::vector<Cell *> allocated;
    std::vector<std::vector<Node *>> results;

    Cell * eos = new_cell(allocated);
    eos->node = __begin_nodes[__sentence.size()][0];
    eos->next = nullptr;
    eos->fx = eos->node->score;
    eos->gx = eos->node->score;
    open_list.push(eos);

    // Run viterbi first to fill the backtrace score, h(x)
    viterbi();

    while (!open_list.empty()) {
        Cell *top = open_list.top();
        open_list.pop();
        Node *node = top->node;

        // Resches to BOS
        if (node == __end_nodes[0][0]) {
            results.resize(results.size() + 1);
            for (Cell *c = top->next; c->next != nullptr; c = c->next) {
                results.back().push_back(c->node);
            }
            if (results.size() == nbest_size) {
                break;
            }
            continue;
        }

        // Expands new node ending at node->pos
        for (Node *lnode : __end_nodes[node->pos]) {
            Cell *cell = new_cell(allocated);
            cell->node = lnode;
            cell->gx = lnode->score + top->gx;
            cell->fx = lnode->backtrace_score + top->gx; // h(x) is backtrace_score
            cell->next = top;
            open_list.push(cell);
        }
    }
    
    util::stl::delete_vector_elements(&allocated);
    return results;
}

float Lattice::populateMarginal(float freq, std::vector<float> *expected) const
{
    if (expected == nullptr) return 0.0;

    const int len = __sentence.size();

    // forward
    std::vector<float> alpha(__all_nodes.size(), 0.0);

    for (int pos = 0; pos <= len; ++pos) {
        for (Node *rnode : __begin_nodes[pos]) {
            for (Node *lnode : __end_nodes[pos]) {
                alpha[rnode->node_id] = log_sum_exp(alpha[rnode->node_id],
                                                    lnode->score + alpha[lnode->node_id],
                                                    lnode == __end_nodes[pos][0]);
            }
        }
    }

    // backward
    std::vector<float> beta(__all_nodes.size(), 0.0);
    for (int pos = len; pos >= 0; --pos) {
        for (Node *lnode : __end_nodes[pos]) {
            for (Node *rnode : __begin_nodes[pos]) {
                beta[lnode->node_id] = log_sum_exp(beta[lnode->node_id],
                                                   rnode->score + beta[rnode->node_id],
                                                   rnode == __begin_nodes[pos][0]);
            }
        }
    }

    //
    const float Z = alpha[__begin_nodes[len][0]->node_id];
    for (int pos = 0; pos < len; ++pos) {
        for (Node *node : __begin_nodes[pos]) {
            if (node->vocab_id >= 0) {
                (*expected)[node->vocab_id] += freq * exp(alpha[node->node_id] + node->score +
                                                          beta[node->node_id] - Z);
            }
        }
    }

    return freq * Z;
}

/////////////////////////////////////////////////////////////////////////////////
Lattice::Node *Lattice::newNode()
{
    Node *node = new Node;
    memset(node, 0, sizeof(*node));
    node->node_id = __all_nodes.size();
    __all_nodes.push_back(node);
    return node;
}