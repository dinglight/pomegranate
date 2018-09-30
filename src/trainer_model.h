#ifndef __TRAINER_MODEL_H__
#define __TRAINER_MODEL_H__
#include <vector>
#include <string>

class TrainerModel {
public:
    void setSentencePieces(std::vector<std::pair<std::u32string, float>> &&sentencepieces);
private:
    std::vector<std::pair<std::u32string, float>> __sentencepieces;
};

#endif // !__TRAINER_MODEL_H__
