#include "trie.h"
#include "test_suite.h"

TEST(TrieTest, commonPrefixSearchTest)
{
    Trie trie;
    const char *keys[] = {
        "bana", "ok", "yup", "banana"
    };
    int32 values[] = {1,2,3,4};
    size_t num = sizeof(values) / sizeof(values[0]);
    trie.build(keys, values, num);

    std::vector<std::pair<std::string, int32>> results;
    trie.commonPrefixSearch("bana", results);

    EXPECT_EQ(2, results.size());
    EXPECT_EQ("bana", results[0].first);
    EXPECT_EQ("banana", results[1].first);

    printf("node count:%d\n", trie.getNodeCount());
}