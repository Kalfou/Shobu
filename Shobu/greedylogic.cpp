#include "greedylogic.h"

#include <QRandomGenerator>

#include "shobuexception.h"

enum GreedyValues
{
    GREEDY_MIN = 2,          // how important is min value related to piece count
    GREEDY_MULTIPLIER = 100  // multiply everything except random with this
};

// PUBLIC

// Constructor
GreedyLogic::GreedyLogic(GameState *state, QObject *parent) : MachineLogic(state, parent){}

// returns the best move to the machineplayer
Move GreedyLogic::getMove()
{
    QVector<Move> moves = _state->getMoves();

    if (moves.isEmpty()) // can not return a legal move when there are no legal moves
    {
        QScopedPointer<ShobuException> exept_ptr(new ShobuException());
        exept_ptr->setMessage("No available moves to select from");
        exept_ptr->raise();
    }

    int index = 0;

    GameState *moved_state = _state->getApplied(moves[0]);

    int max = evaluateState(moved_state);

    for (int i = 1; i < moves.length(); ++i) // find move with the highest score
    {
        delete moved_state;
        moved_state = _state->getApplied(moves[i]);

        int score = evaluateState(moved_state);
        if (score > max)
        {
            index = i;
            max = score;
        }
    }
    delete moved_state;
    return moves[index];
}

// PRIVATE

// gives a score to a given state
int GreedyLogic::evaluateState(const GameState *state)
{
    int score = 20; // the opponent has at most 16 pieces with a minimum of 4 per board

    Color opponent = state->getTurn(); // the current player after our move is the opponent

    int min = 5;
    int index = 0;

    // count pieces on each boaed for both players
    for (int i = 0; i < 4; ++i)
    {
        int board_min = 0; // find board with the fewest opposing pieces
        for (int j = 0; j < 4; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                if (state->getField(i,j,k) == opponent)
                {
                    --score;
                    ++board_min;
                }
            }
        }
        if (min > board_min || (min > board_min && state->isHomeBoard(opponent, i)))
        {
            min = board_min;
            index = i;
        }
    }
    score -= (min*GREEDY_MIN); // always hit in the board closest to victory

    score = score * GREEDY_MULTIPLIER;

    if (state->isHomeBoard(opponent, index)) // score for preferred board is less than a piece score
    {
        score += GREEDY_MULTIPLIER/2;
    }

    score += QRandomGenerator::global()->bounded(0,GREEDY_MULTIPLIER/2); // random has to be less than any relevant score

    return score;
}
