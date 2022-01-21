#include "hardlogic.h"

#include <QRandomGenerator>

#include "shobuexception.h"

enum EvaluateValues
{
    UNREACHABLE = 1000000000, // initial value in min or max search
    MAX_SCORE   =   10000000, // score for victory or defeat
    PIECE_VALUE =        100, // score for one piece on a board
    HOME_BONUS  =         10, // score for having one of the opponent's homeboard as the weakest
    WEAKEST     =         10, // multiplier for the weakest board piece penalty
    RAND_BOUND  =          5  // exclusive maximum of random value given to the score
};

// PUBLIC

// Constructor
HardLogic::HardLogic(GameState *state, Color color, QObject *parent) : MachineLogic(state, parent), _side(color)
{
    _opponent = _state->getOpponent(_side);
}

// returns the best move to the machineplayer
Move HardLogic::getMove()
{
    QVector<Move> moves = _state->getMoves();

    if (moves.isEmpty()) // can not return a legal move when there are no legal moves
    {
        QScopedPointer<ShobuException> exept_ptr(new ShobuException());
        exept_ptr->setMessage("No available moves to select from");
        exept_ptr->raise();
    }

    int index = -1;
    int max = -UNREACHABLE; // initial minimum must always be surpassed

    for (int i = 0; i < moves.length(); ++i)
    {
        ReverseData reverse = _state->applyMove(moves[i]);
        int score = evaluateState();
        if (score > max)
        {
            index = i;
            max = score;
        }
        _state->reverseMove(moves[i], reverse);
    }

    return moves[index];
}

// PRIVATE

// gives a score to a given state
int HardLogic::evaluateState()
{
    int score = 0;

    int piece_count[4][2] = { // pieces
        {0,0},
        {0,0},
        {0,0},
        {0,0}};

    // count pieces on each board for both players
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                if (Color color = _state->getField(i,j,k); color != EMPTY)
                {
                    // count piece
                    ++piece_count[i][color];

                    // add position value to score
                    if (color == _side)
                    {
                        score += PIECE_VALUE;
                        if (GameState::isHomeBoard(color, i))
                        {
                            score += sideHomeValues[j][k];
                        }
                        else
                        {
                            score += sideOpposingValues[j][k];
                        }
                    }
                    else
                    {
                        score += -PIECE_VALUE;
                        if (GameState::isHomeBoard(color, i))
                        {
                            score += opponentHomeValues[j][k];
                        }
                        else
                        {
                            score += opponentOpposingValues[j][k];
                        }
                    }
                }
            }
        }
    }

    // find minimum piece count of each player
    int color_min[2] = {0,0};

    for (int i = 1; i < 4; ++i)
    {
        if (piece_count[color_min[_side]][_side] > piece_count[i][_side])
        {
            color_min[_side] = i;
        }
        if (piece_count[color_min[_opponent]][_opponent] > piece_count[i][_opponent])
        {
            color_min[_opponent] = i;
        }
        else if (piece_count[color_min[_opponent]][_opponent] == piece_count[i][_opponent] && _state->isHomeBoard(_opponent, i))
        {
            color_min[_opponent] = i; // we prefer the homeboard to be the weakest
        }
    }

    if (piece_count[color_min[_opponent]][_opponent] == 0) // victory is always the best option
    {
        return MAX_SCORE;
    }

    if (piece_count[color_min[_side]][_side] == 1) // do not pick moves ending in our defeat
    {
        QVector<Move> moves = _state->getMoves();

        for (int i = 0; i < moves.length(); ++i) // check all possible moves from opponent
        {
            ReverseData reverse = _state->applyMove(moves[i]);
            if (_state->getVictor() != EMPTY) // if opponent can win, this move is bad
            {
                _state->reverseMove(moves[i], reverse);
                return -MAX_SCORE;
            }
            _state->reverseMove(moves[i], reverse);
        }
    }

    score += -piece_count[color_min[_opponent]][_opponent]*WEAKEST; // add penalty for pieces on opponent's weakest board

    if (_state->isHomeBoard(_opponent, color_min[_opponent])) // award if opponent homeboard is the weakest
    {
        score += HOME_BONUS;
    }

    score += QRandomGenerator::global()->bounded(0,RAND_BOUND); // random has to be less than any relevant score

    return score;
}
