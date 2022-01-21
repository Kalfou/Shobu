#ifndef SHOBUPLAYER_H
#define SHOBUPLAYER_H

#include <QObject>

#include "gameutils.h"

struct Move;
struct MoveState;

class ShobuPlayer : public QObject
{
    Q_OBJECT
public:
    ShobuPlayer(MoveState *m_state, Color color, QObject *parent = nullptr) : QObject(parent), state(m_state), side(color){};

    virtual void makeMove(int, int, int){};
    virtual void makeMove(Move) = 0;
    virtual void makeMove() = 0;

protected:
    MoveState *state;
    Color side;

signals:
    void moveMade();
    void moveProgress();
};

#endif // SHOBUPLAYER_H
