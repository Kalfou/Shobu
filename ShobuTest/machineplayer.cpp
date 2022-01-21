#include "machineplayer.h"

#include "randomlogic.h"
#include "greedylogic.h"
#include "hardlogic.h"
#include "shobuexception.h"

// PUBLIC

// Constructor
MachinePlayer::MachinePlayer(MoveState *m_state, Color color, Difficulty difficulty, QObject *parent) : ShobuPlayer(m_state, color, parent)
{
    switch (difficulty)
    {
    case EASY:
        _logic = new RandomLogic(state->game, this);
        break;
    case MEDIUM:
        _logic = new GreedyLogic(state->game, this);
        break;
    case HARD:
        _logic = new HardLogic(state->game, color, this);
        break;
    default:
        _logic = new RandomLogic(state->game, this);
        break;
    }
}

// gets a move form the machinelogic and notifies the logic
void MachinePlayer::makeMove()
{
    state->move = _logic->getMove();
    state->passive_set = true;
    state->vector_set = true;
    emit moveMade();
}

void MachinePlayer::makeMove(Move) {} // machineplayer does not accept input
