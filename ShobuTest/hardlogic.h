#ifndef HARDLOGIC_H
#define HARDLOGIC_H

#include "machinelogic.h"

class HardLogic : public MachineLogic
{
    Q_OBJECT
public:
    HardLogic(GameState *state, Color color, QObject *parent = nullptr);

    Move getMove() override;

private:
    Color _side, _opponent;

    int evaluateState();

    // position values
    int sideHomeValues[4][4] =
    {
        {20, 25, 25, 20},
        {25, 30, 30, 25},
        {25, 30, 30, 25},
        {20, 25, 25, 20}
    };
    int sideOpposingValues[4][4] =
    {
        {20, 30, 30, 20},
        {30, 40, 40, 30},
        {30, 40, 40, 30},
        {20, 30, 30, 20}
    };

    int opponentHomeValues[4][4] =
    {
        {-20, -30, -30, -20},
        {-30, -40, -40, -30},
        {-30, -40, -40, -30},
        {-20, -30, -30, -20}
    };
    int opponentOpposingValues[4][4] =
    {
        {-20, -25, -25, -20},
        {-25, -30, -30, -25},
        {-25, -30, -30, -25},
        {-20, -25, -25, -20}
    };
};

#endif // HARDLOGIC_H
