#ifndef RANDOMLOGIC_H
#define RANDOMLOGIC_H

#include "machinelogic.h"

class RandomLogic : public MachineLogic
{
    Q_OBJECT
public:
    RandomLogic(GameState *state, QObject *parent = nullptr) : MachineLogic(state, parent){}

    Move getMove() override;
};

#endif // RANDOMLOGIC_H
