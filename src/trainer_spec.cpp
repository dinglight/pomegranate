#include "trainer_spec.h"

TrainerSpec::TrainerSpec()
    : __num_sub_iterations(2)
{
}

TrainerSpec::~TrainerSpec()
{
}

void TrainerSpec::addTrainFile(const std::string& value)
{
    __train_files.push_back(value);
}
const std::vector<std::string>& TrainerSpec::getTrainFiles() const
{
    return __train_files;
}