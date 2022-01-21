#include "forwardthinkerlogic.h"

#include <QDebug>

enum ForwardThinkerValues
{
    DEPTH           =       2,  // the depth of the search tree
    MAXIMUM_INIT    = 1000000,  // initialize alpha and beta; no score can exceed this
    VICTORY         =  100000,  // score for victory and defeat; only alpha and beta exceeds this
    MIN_MULTIPLIER  =       2,  // how important is min value related to piece count
    MULTIPLIER      =     100,  // multiply everything except random with this
};

// Constructor
ForwardThinkerLogic::ForwardThinkerLogic(GameState *state, Color color, QObject *parent) : MachineLogic(state, parent)
{
    side = color;
    opponent = side == WHITE ? BLACK : WHITE;
    test = 0;
}

// returns the best move to the machineplayer
Move ForwardThinkerLogic::getMove()
{
    QVector<Move> moves = _state->getMoves();

    if(moves.isEmpty())
    {
        throw "No available moves to select from";
    }

    int index = -1;


    int max = -MAXIMUM_INIT;
    /*int alpha = max;
    int beta = -max;*/

    for (int i = 0; i < moves.length(); ++i)
    {

        //GameState *moved_state = _state->getApplied(moves[i]);
        ReverseData reverse = _state->applyMove(moves[i]);
        //int score = evaluateState(1, DEPTH-1, alpha, beta);
        int score = alphaBeta(DEPTH - 1, true, -MAXIMUM_INIT, MAXIMUM_INIT);
        if(score > max)
        {
            index = i;
            max = score;
        }
        _state->reverseMove(moves[i], reverse);
    }
    qDebug()<<test;
    test = 0;
    return moves[index];
}

// gives a score to the current state
int ForwardThinkerLogic::evaluateState(int sign,  int level, int alpha, int beta)
{
    if(_state->getVictor() == _state->getTurn())
    {
        return VICTORY * sign; // highest possible return value
    }
    QVector<Move> moves = _state->getMoves();

    if(moves.isEmpty())
    {
        return VICTORY * sign; // highest possible return value
    }

    if (level < 1)
    {
        return evaluateLeaf() * sign;
    }


    int max = MAXIMUM_INIT * sign;


    for (int i = 0; i < moves.length(); ++i)
    {

        //GameState *moved_state = _state->getApplied(moves[i]);
        ReverseData reverse = _state->applyMove(moves[i]);

        int score = evaluateState(sign * -1, level - 1, alpha, beta) * sign;
        //qDebug()<<score;
        if(score > max)
        {

            max = score;
        }
        _state->reverseMove(moves[i], reverse);
        //delete moved_state;
    }
    return max * sign;
}

// uses alpha beta algorithm to find move with the best score
int ForwardThinkerLogic::alphaBeta(int level, bool is_maxing, int alpha, int beta)
{
    if(Color victor = _state->getVictor(); victor != EMPTY)
    {
        if(victor == side)
        {
            return VICTORY;
        }
        else
        {
            return -VICTORY;
        }
    }

    QVector<Move> moves = _state->getMoves();

    if(moves.isEmpty())
    {
        if(_state->getTurn() == side)
        {
            return -VICTORY;
        }
        else
        {
            return VICTORY;
        }
    }

    if (level < 1)
    {
        return evaluateLeaf();
    }

    int score;
    if(is_maxing)
    {
        score = -MAXIMUM_INIT;
        for (int i = 0; i < moves.length(); ++i)
        {
            ReverseData reverse = _state->applyMove(moves[i]);
            int value = alphaBeta(level - 1, !is_maxing, alpha, beta);
            score = value > score ? value : score;
            alpha = score > alpha ? score : alpha;
            if(alpha >= beta)
            {
                i = moves.length();
            }
            _state->reverseMove(moves[i], reverse);
        }
    }
    else
    {
        score = MAXIMUM_INIT;
        for (int i = 0; i < moves.length(); ++i)
        {
            ReverseData reverse = _state->applyMove(moves[i]);
            int value = alphaBeta(level - 1, !is_maxing, alpha, beta);
            score = value < score ? value : score;
            alpha = score < alpha ? score : alpha;
            if(alpha >= beta)
            {
                i = moves.length();
            }
            _state->reverseMove(moves[i], reverse);
        }
    }
    return score;
}

// gives a score to the current state
int ForwardThinkerLogic::evaluateLeaf()
{
    ++test;
    if (Color victor = _state->getVictor(); victor != EMPTY)
    {
        if(victor == side)
        {
            return 1000000; // victory should always be selected
        }
        else
        {
            return -1000000; // defeat must always be avoided
        }
    }

    int score = 20;


    //initialize pieces at 0
    int piece_count[2][4] = {
        {0,0,0,0},
        {0,0,0,0}
    };
    /*int black_min = 0;
    int white_min = 0;*/
    //int min = 5;
    //int index = 0;

    // count pieces on each board for both players
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            //int board_min = 0;
            for (int k = 0; k < 4; ++k)
            {
                if (_state->getField(i,j,k) != EMPTY)
                {
                    ++piece_count[_state->getField(i,j,k)][i];

                    if(_state->getField(i,j,k) == side)
                    {
                        if(_state->isHomeBoard(side, i))
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
                        if(_state->isHomeBoard(side, i))
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
    /*score -= min*MIN_MULTIPLIER;
    if(_state->isHomeBoard(opponent, index))
    {
        ++score;
    }
    score = score * MULTIPLIER;*/

    return score;
}

