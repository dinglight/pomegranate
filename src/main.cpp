#include "trainer.h"

int main()
{
    TrainerSpec trainer_spec;
    //trainer_spec.addTrainFile("pku_test.utf8");
    trainer_spec.addTrainFile("corpus.txt");

    Trainer trainer(trainer_spec);
    trainer.train();
}