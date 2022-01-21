#include "organicplayer.h"

#include "gamestate.h"

// PUBLIC

// updates the movestate based on user input
void OrganicPlayer::makeMove(int board, int row, int column)
{
    if (state->passive_set)
    {
        if (state->vector_set) // set agressive
        {
            state->move.a = Coordinate(board, row, column);

            if (state->game->isLegalAgressive(state->move)) // everything is set, ready to make the move
            {
                emit moveMade();
            }
        }
        else if (state->move.p.board == board)  // set vector
        {
            //full change (direction and magnitude)
            state->move.row_change = row - state->move.p.row;
            state->move.col_change = column - state->move.p.column;

            //get signed magnitude
            int row_signed = state->move.row_change > 0 ? state->move.row_change : -state->move.row_change;
            int col_signed = state->move.col_change > 0 ? state->move.col_change : -state->move.col_change;

            if ((row_signed == col_signed && row_signed != 0) || (row_signed*col_signed == 0 && row_signed+col_signed != 0))
            {
                // get absolute magnitude
                state->move.magnitude  = row_signed > 0 ? row_signed : col_signed;

                //keep direction only
                state->move.row_change = state->move.row_change/state->move.magnitude;
                state->move.col_change = state->move.col_change/state->move.magnitude;

                if (state->game->isLegalVector(state->move.p, state->move.row_change, state->move.col_change, state->move.magnitude))
                {
                    // we accept the given move vector
                    state->vector_set = true;
                    emit moveProgress();
                }
            }
        }
    }
    else // set passive
    {
        state->move.p = Coordinate(board, row, column);

        if (state->game->isLegalPassive(state->move.p))
        {
            // we accept the passive piece
            state->passive_set = true;
            emit moveProgress();
        }
    }
}

// notifies the model of a new move
void OrganicPlayer::makeMove(Move move)
{
    state->move        = move;
    state->passive_set = true;
    state->vector_set  = true;
    emit moveMade();
}
