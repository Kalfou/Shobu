#ifndef FORWARDTHINKERLOGIC_H
#define FORWARDTHINKERLOGIC_H

#include "machinelogic.h"

class ForwardThinkerLogic : public MachineLogic
{
    Q_OBJECT
public:
    ForwardThinkerLogic(GameState *state, Color color, QObject *parent = nullptr);

    Move getMove() override;

private:
    Color side, opponent;
    int test;

    int evaluateState(int sign, int level, int alpha, int beta);
    int alphaBeta(int level, bool is_maxing, int alpha, int beta);
    int evaluateLeaf();


    // position values
    const int sideHomeValues[4][4] =
    {
        {20, 25, 25, 20},
        {25, 30, 30, 25},
        {25, 30, 30, 25},
        {20, 25, 25, 20}
    };
    const int sideOpposingValues[4][4] =
    {
        {20, 30, 30, 20},
        {30, 40, 40, 30},
        {30, 40, 40, 30},
        {20, 30, 30, 20}
    };

    const int opponentHomeValues[4][4] =
    {
        {-20, -30, -30, -20},
        {-20, -40, -40, -20},
        {-20, -40, -40, -20},
        {-20, -30, -30, -20}
    };
    const int opponentOpposingValues[4][4] =
    {
        {-20, -25, -25, -20},
        {-20, -30, -30, -20},
        {-20, -30, -30, -20},
        {-20, -25, -25, -20}
    };
};

#endif // FORWARDTHINKERLOGIC_H
