#include "lattice.h"
#include "test_suite.h"
#include "util.h"


static void insert_node_with_score(Lattice *lattice, int pos, int length, float score)
{
    lattice->insertNode(pos, length)->score = score;
}

static void insert_node_with_score_id(Lattice *lattice, int pos, int length, float score,
                                      int id)
{
    auto *node = lattice->insertNode(pos, length);
    node->score = score;
    node->vocab_id = id;
}

static std::string get_tokenized(const std::vector<Lattice::Node *> &nodes)
{
    std::vector<std::string> tokens;
    for (auto *node : nodes) {
        tokens.push_back(util::string::unicodetext_to_utf8text(node->piece));
    }
    return util::string::join(tokens, " ");
}

TEST(LatticeTest, ViterbiTest)
{
    Lattice lattice;
    std::u32string sentence = util::string::utf8text_to_unicodetext("ABC");
    lattice.setSentence(sentence);

    insert_node_with_score(&lattice, 0, 1, 0.0);  // A
    insert_node_with_score(&lattice, 1, 1, 0.0);  // B
    insert_node_with_score(&lattice, 2, 1, 0.0);  // C
    EXPECT_EQ("A B C", get_tokenized(lattice.viterbi()));

    insert_node_with_score(&lattice, 0, 2, 2.0);  // AB
    EXPECT_EQ("AB C", get_tokenized(lattice.viterbi()));

    insert_node_with_score(&lattice, 1, 2, 5.0);  // BC
    EXPECT_EQ("A BC", get_tokenized(lattice.viterbi()));

    insert_node_with_score(&lattice, 0, 3, 10.0);  // ABC
    EXPECT_EQ("ABC", get_tokenized(lattice.viterbi()));
}

TEST(LatticeTest, NBestTest)
{
    Lattice lattice;
    std::u32string sentence = util::string::utf8text_to_unicodetext("ABC");
    lattice.setSentence(sentence);

    insert_node_with_score(&lattice, 0, 1, 0.0);  // A
    insert_node_with_score(&lattice, 1, 1, 0.0);  // B
    insert_node_with_score(&lattice, 2, 1, 0.0);  // C
    insert_node_with_score(&lattice, 0, 2, 2.0);  // AB
    insert_node_with_score(&lattice, 1, 2, 5.0);  // BC
    insert_node_with_score(&lattice, 0, 3, 10.0);  // ABC

    auto nbests = lattice.nbest(10);
    EXPECT_EQ(4, nbests.size());

    EXPECT_EQ("ABC", get_tokenized(nbests[0]));
    EXPECT_EQ("A BC", get_tokenized(nbests[1]));
    EXPECT_EQ("AB C", get_tokenized(nbests[2]));
    EXPECT_EQ("A B C", get_tokenized(nbests[3]));
}

TEST(LatticeTest, PopulateMarginalTest)
{
    Lattice lattice;
    std::u32string sentence = util::string::utf8text_to_unicodetext("ABC");
    lattice.setSentence(sentence);

    insert_node_with_score_id(&lattice, 0, 1, 1.0, 0);  // A
    insert_node_with_score_id(&lattice, 1, 1, 1.2, 1);  // B
    insert_node_with_score_id(&lattice, 2, 1, 2.5, 2);  // C
    insert_node_with_score_id(&lattice, 0, 2, 3.0, 3);  // AB
    insert_node_with_score_id(&lattice, 1, 2, 4.0, 4);  // BC
    insert_node_with_score_id(&lattice, 0, 3, 2.0, 5);  // ABC

    std::vector<float> probs(6, 0.0);

    // Expand all paths:
    // A B C : exp(1.0 + 1.2 + 2.5) => path1
    // AB  C : exp(3.0 + 2.5)       => path2
    // A  BC : exp(1.0 + 4.0)       => path3
    // ABC   : exp(2.0)             => path4
    const float p1 = exp(1.0 + 1.2 + 2.5);
    const float p2 = exp(3.0 + 2.5);
    const float p3 = exp(1.0 + 4.0);
    const float p4 = exp(2.0);
    const float Z = p1 + p2 + p3 + p4;

    const float logZ = lattice.populateMarginal(1.0, &probs);

    EXPECT_NEAR((p1 + p3) / Z, probs[0], 0.001);  // A
    EXPECT_NEAR(p1 / Z, probs[1], 0.001);         // B
    EXPECT_NEAR((p1 + p2) / Z, probs[2], 0.001);  // C
    EXPECT_NEAR(p2 / Z, probs[3], 0.001);         // AB
    EXPECT_NEAR(p3 / Z, probs[4], 0.001);         // BC
    EXPECT_NEAR(p4 / Z, probs[5], 0.001);         // ABC
    EXPECT_NEAR(log(Z), logZ, 0.001);
}