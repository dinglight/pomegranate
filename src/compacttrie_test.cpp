#include "compacttrie.h"
#include "test_suite.h"
#include <algorithm>

template <typename K, typename V>
std::vector<std::pair<K, V>> Sorted(const std::vector<std::pair<K, V>> &m)
{
    std::vector<std::pair<K, V>> v = m;
    std::sort(v.begin(), v.end(),
              [](const std::pair<K, V> &p1, const std::pair<K, V> &p2) {
        return (p1.second > p2.second ||
            (p1.second == p2.second && p1.first < p2.first));
    });
    return v;
}

TEST(CompatTrieTest, commonPrefixSearchTest)
{
    CompactTrie trie;
    const char *keys[] = {
        "interest", "ok", "inside", "interested", "interesting"
    };
    int32 values[] = {1,2,3,4,5};
    size_t num = sizeof(values) / sizeof(values[0]);
    trie.build(keys, values, num);
    EXPECT_EQ(7, trie.getNodeCount());

    std::vector<std::pair<std::string, int32>> results;
    trie.commonPrefixSearch("in", results);

    results = Sorted(results);

    EXPECT_EQ(4, results.size());
    EXPECT_EQ("interest", results[3].first);
    EXPECT_EQ("interested", results[1].first);
    EXPECT_EQ("interesting", results[0].first);
    EXPECT_EQ("inside", results[2].first);
}