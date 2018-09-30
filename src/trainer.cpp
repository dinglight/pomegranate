#include "trainer.h"

#include <fstream>
#include <algorithm>

#include "third_party/esaxx/esa.hxx"
#include "lattice.h"

#include "util.h"
#include "log.h"

template <typename K, typename V>
std::vector<std::pair<K, V>> sort_vector_by_second(const std::vector<std::pair<K, V>> &m)
{
    std::vector<std::pair<K, V>> v = m;
    std::sort(v.begin(), v.end(),
              [](const std::pair<K, V> &p1, const std::pair<K, V> &p2) {
        return (p1.second > p2.second ||
            (p1.second == p2.second && p1.first < p2.first));
    });
    return v;
}

template <typename K, typename V>
std::vector<std::pair<K, V>> sort_map_by_second(const std::unordered_map<K, V> &m)
{
    std::vector<std::pair<K, V>> v(m.begin(), m.end());
    return sort_vector_by_second(v);
}

template <typename IT>
void to_logprob(IT begin, IT end)
{
    float sum = 0.0;
    for (auto it = begin; it != end; ++it) {
        sum += it->second;
    }
    float logsum = log(sum);
    for (auto it = begin; it != end; ++it) {
        it->second = log(it->second) - logsum;
    }
}

Trainer::Trainer(const TrainerSpec &trainer_spec)
    :__trainer_spec(trainer_spec)
{

}

Trainer::~Trainer()
{

}

bool Trainer::train()
{
    RETURN_IF_FALSE(loadSentences());
    __sentencepieces = std::move(makeSeedSentencePieces());
    
    LOG(I, "Using %d sentences for EM training", __sentences.size());

    while (true) {
        // EM iteration
        for (int iter = 0; iter < __trainer_spec.num_sub_iterations(); ++iter) {
            // Executes E step
            float objective = 0.0;
            int64 num_tokens = 0;
            const auto expected = runEStep(&objective, &num_tokens);

            // Executes M step
            auto new_sentencepieces = runMStep(expected);
            __sentencepieces = std::move(new_sentencepieces);
        }
    }
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
bool Trainer::loadSentences()
{
    for (const auto &filename : __trainer_spec.getTrainFiles()) {
        std::ifstream is(filename.c_str());
        std::string line;
        int64 freq = 1;
        while (std::getline(is, line)) {
            std::u32string unicode_line = util::string::utf8text_to_unicodetext(line);
            __sentences.emplace_back(unicode_line, freq);
        }
        is.close();
    }

    std::unordered_map<char32, int64> chars_count;
    for (const auto &w : __sentences) {
        for (const char32 c : w.first) {
            if (!util::string::is_valid_codepoint(c)) continue;
            if (c == kUnicodeNone) continue;
            if (c == kUnicodeSpace) continue;
            chars_count[c] += w.second;
        }
    }

    for (const auto &w : sort_map_by_second(chars_count)) {
        __required_chars.insert(w);
    }

    return true;
}

std::vector<std::pair<std::u32string, float>> Trainer::makeSeedSentencePieces() const
{
    // merge all sentences into one array with 0x0000 delimiter.
    std::vector<char32> array;
    std::unordered_map<char32, int64> all_chars;
    constexpr char32 kSentenceBoundary = 0x0000;

    const size_t mining_size = __sentences.size();
    std::vector<std::u32string> mining_sentences(mining_size);
    for (size_t i = 0; i < mining_size; ++i) {
        mining_sentences[i] = __sentences[i].first;
    }

    for (const auto &w : mining_sentences) {
        for (const auto &c : w) {
            array.push_back(c);
            if (c != kSentenceBoundary) {
                ++all_chars[c];
            }
        }
        array.push_back(kSentenceBoundary);
    }

    // make a suffix array to extract all sub strings occurring
    // more than 2 times in the sentence.
    const int n = array.size();
    std::vector<int> SA(n);         // suffix array
    std::vector<int> L(n);          // left boundaries of internal node
    std::vector<int> R(n);          // right boundaries of internal node
    std::vector<int> D(n);          // depths of internal node
    constexpr int kAlphabetSize = 0x110000; // All UCS4 range.
    int node_num = 0;
    LOG(I, "Make");
    esaxx(array.begin(), SA.begin(), L.begin(), R.begin(), D.begin(),
          n, kAlphabetSize, node_num);

    std::vector<std::pair<int, int>> substr_index;
    for (int i = 0; i < node_num; ++i) {
        const int offset = SA[L[i]];
        const int len = D[i];
        // skip substring that length < 2 
        if (len <= 1) {
            continue;
        }
        const char32 *begin = &array[0] + offset;
        const char32 *end = &array[0] + offset + len;
        // skip substring that contain a sentence boundary.
        if (std::find(begin, end, kSentenceBoundary) != end) {
            continue;
        }

        // skip substring that has invalid unicode

        // 
        const int freq = R[i] - L[i];
        const int score = freq * len;
        substr_index.emplace_back(i, score);
    }

    //
    std::vector<std::pair<std::u32string, float>> seed_sentencepieces;
    for (const auto &it : sort_map_by_second(all_chars)) {
        std::u32string uw = { it.first };
        seed_sentencepieces.emplace_back(uw, static_cast<float>(it.second));
    }

    // Sort by the coverage of sub strings.
    for (const auto &p : sort_vector_by_second(substr_index)) {
        const int offset = SA[L[p.first]];
        const int len = D[p.first];
        if (len < 0) {
            continue;
        }
        const char32 *begin = &array[offset];
        const char32 *end = &array[offset + len];
        const std::u32string uw(begin, end);
        seed_sentencepieces.emplace_back(uw, static_cast<float>(p.second));
    }

    to_logprob(seed_sentencepieces.begin(), seed_sentencepieces.end());

    LOG(I, "Initialized %d seed sentencepieces", seed_sentencepieces.size());

    return seed_sentencepieces;
}

std::vector<float> Trainer::runEStep(float *obj, int64 *num_tokens) const
{
    std::vector<float> expected;

    int64 all_sentence_freq = 0;
    for (const auto &w : __sentences) {
        all_sentence_freq += w.second;
    }

    // Executes E Step
    Lattice lattice;
    expected.resize(, 0.0);

}

void Trainer::buildTrie(std::vector<std::pair<std::u32string, int>> *pieces)
{
    // sort by sentencepiece since DoubleArray::build()
    // only accepts sorted strings.
    sort(pieces->begin(), pieces->end());

    //
    const size_t pieces_size = pieces->size();
    std::vector<const char32 *> key(pieces_size);
    std::vector<int> value(pieces_size);
    for (size_t i = 0; i < pieces_size; ++i) {
        key[i] = (*pieces)[i].first.data();
        value[i] = (*pieces)[i].second;
    }

    __trie = std::unique_ptr<Darts::DoubleArray>(new Darts::DoubleArray);
    __trie->build(key.size(), )
}