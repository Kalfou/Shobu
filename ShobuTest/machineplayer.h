#ifndef MACHINEPLAYER_H
#define MACHINEPLAYER_H

#include <QObject>

#include "shobuplayer.h"

class MachineLogic;
struct Move;

class MachinePlayer : public ShobuPlayer
{
    Q_OBJECT
public:
    MachinePlayer(MoveState *m_state, Color color, Difficulty difficulty, QObject *parent = nullptr);

    void makeMove(int, int, int) override {makeMove();};
    void makeMove(Move) override;
    void makeMove() override;

private:
    MachineLogic *_logic;
};

#endif // MACHINEPLAYER_H
