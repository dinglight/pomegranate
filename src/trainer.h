#ifndef __TRAINER_H__
#define __TRAINER_H__


#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

#include "third_party/darts_clone/darts.h"
#include "common.h"
#include "trainer_spec.h"

class Trainer {
public:
    Trainer(const TrainerSpec &trainer_spec);
    virtual ~Trainer();
    bool train();

private:
    bool loadSentences();

    // Make seed pieces from the training corpus
    std::vector<std::pair<std::u32string, float>> makeSeedSentencePieces() const;

    //
    std::vector<float> runEStep(float *obj, int64 *num_tokens) const;


    // 
    void buildTrie(std::vector<std::pair<std::u32string, int>> *pieces);
private:
    std::vector<std::pair<std::u32string, int64>> __sentences;
    std::unordered_map<char32, int64> __required_chars;

    std::vector<std::pair<std::u32string, float>> __sentencepieces;
    std::unique_ptr<Darts::DoubleArray> __trie;

    TrainerSpec __trainer_spec;
};

#endif // !__TRAINER_H__

