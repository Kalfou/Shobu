#ifndef MACHINELOGIC_H
#define MACHINELOGIC_H

#include <QObject>

#include "gamestate.h"

class MachineLogic : public QObject
{
    Q_OBJECT
public:
    MachineLogic(GameState *state, QObject *parent = nullptr) : QObject(parent), _state(state) {};

    virtual Move getMove() = 0;

protected:
    GameState *_state;
};

#endif // MACHINELOGIC_H
