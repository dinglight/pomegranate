#ifndef __TRAINER_SPEC_H__
#define __TRAINER_SPEC_H__
#include <string>
#include <vector>
#include "common.h"

class TrainerSpec {
public:
    TrainerSpec();
    virtual ~TrainerSpec();

    inline int32 num_sub_iterations() const
    {
        return __num_sub_iterations;
    }

    void addTrainFile(const std::string& value);
    const std::vector<std::string>& getTrainFiles() const;


private:
    int32 __num_sub_iterations;
    std::vector<std::string> __train_files;
};



#endif // !__TRAINER_SPEC_H__
