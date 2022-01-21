#include "randomlogic.h"

#include <QRandomGenerator>

#include "shobuexception.h"

// PUBLIC

// returns a random move
Move RandomLogic::getMove()
{
    QVector<Move> moves = _state->getMoves();

    if (moves.isEmpty()) // can not return a legal move when there are no legal moves
    {
        QScopedPointer<ShobuException> exept_ptr(new ShobuException());
        exept_ptr->setMessage("No available moves to select from");
        exept_ptr->raise();
    }

    int index = QRandomGenerator::global()->bounded(moves.length());
    return moves[index];
}
