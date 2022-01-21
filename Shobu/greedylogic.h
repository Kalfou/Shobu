#ifndef GREEDYLOGIC_H
#define GREEDYLOGIC_H

#include "machinelogic.h"

class GreedyLogic : public MachineLogic
{
    Q_OBJECT
public:
    GreedyLogic(GameState *state, QObject *parent = nullptr);

    Move getMove() override;

private:
    int evaluateState(const GameState *state);
};

#endif // GREEDYLOGIC_H
