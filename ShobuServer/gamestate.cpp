#include "gamestate.h"

#include <QScopedPointer>

#include "shobuexception.h"

// PUBLIC

// set up initial gamestate
void GameState::initializeGame()
{
    // Empty all
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                _board[i][j][k] = EMPTY;
            }
        }
    }
    // Place pieces on first and last row on each board
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            _board[i][0][j] = BLACK;
            _board[i][3][j] = WHITE;
        }
    }
    // White always starts the game
    _turn = WHITE;
}

// determines if board with the given index is homeboard of given color
bool GameState::isHomeBoard(Color color, int board_id)
{
    if (color == BLACK && board_id >=0 && board_id < 2)
    {
        return true;
    }
    if (color == WHITE && board_id >=2 && board_id < 4)
    {
        return true;
    }
    return false;
}

// Setters
// copy state of another GameState
void GameState::setState(const GameState *from_state)
{
    if (from_state == nullptr)
    {
        QScopedPointer<ShobuException> exept_ptr(new ShobuException());
        exept_ptr->setMessage("Nullpointer exception");
        exept_ptr->raise();
    }

    // Copy all positions by value
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                _board[i][j][k] = from_state->_board[i][j][k];
            }
        }
    }

    // Copy turn
    _turn = from_state->_turn;
}

// field setter
void GameState::setField(int table, int row, int column, Color color)
{
    if (table < 0 || table > 3 || !onBoard(row, column))
    {
        QScopedPointer<ShobuException> exept_ptr(new ShobuException());
        exept_ptr->setMessage("Field does not exist");
        exept_ptr->raise();
    }
    _board[table][row][column] = color;
}

// turn setter
void GameState::setTurn(Color color)
{
    if (color == EMPTY)
    {
        QScopedPointer<ShobuException> exept_ptr(new ShobuException());
        exept_ptr->setMessage("Turn has to be BLACK or WHITE");
        exept_ptr->raise();
    }
    _turn = color;
}

// Step functions
// apply a move to the current board if it is legal
void GameState::makeMove(Move move)
{
    if (isLegalMove(move))
    {
        applyMove(move);
    }
}

// give the turn to the next player
void GameState::endTurn()
{
    _turn = getOpponent(); // will be white if unitialized
}

// check if someone won the game on a board after a turn
Color GameState::getVictor() const
{
    // count white and black pieces on each board
    for (int i = 0; i < 4; ++i)
    {
        int whites = 0;
        int blacks = 0;
        for (int j = 0; j < 4; ++j) // count black and white pieces
        {
            for (int k = 0; k < 4; ++k)
            {
                switch (_board[i][j][k])
                {
                case WHITE:
                    ++whites;
                    break;
                case BLACK:
                    ++blacks;
                    break;
                default:
                    break;
                }
            }
        }
        // if one color is completely gone from a board, game over
        if (!blacks)
        {
            return WHITE;
        }
        if (!whites)
        {
            return BLACK;
        }
    }
    return EMPTY; // otherwise noone won yet
}

// Move checkers
// checks if the opponent has any valid moves
bool GameState::hasMoves() const
{
    return !getMoves().isEmpty();
}

// checks all possible illegal moves
bool GameState::isLegalMove(Move move) const
{
    return isLegalPassive(move.p)
           && isLegalVector(move.p, move.row_change, move.col_change, move.magnitude)
           && isLegalAgressive(move);
}

// checks if passive piece is legal if nothing else is set
bool GameState::isLegalPassive(Coordinate coord) const
{
    // passive piece is on a legal field
    if (coord.board < 0 || coord.board > 3 || !onBoard(coord.row, coord.column))
    {
        return false;
    }

    // player controls the passive field
    if (_board[coord.board][coord.row][coord.column] != _turn)
    {
        return false;
    }

    // Passive field is on homeboard
    if ((_turn == WHITE && coord.board < 2) || (_turn == BLACK && coord.board > 1))
    {
        return false;
    }

    return true;
}

// checks if vector is legal assuming passive piece is legal and agressive is not yet set
bool GameState::isLegalVector(Coordinate p, int row_change, int col_change, int magnitude) const
{
    // valid magnitude
    if (magnitude != 1 && magnitude != 2)
    {
        return false;
    }

    // valid row_change
    if (row_change > 1 || row_change < -1)
    {
        return false;
    }

    // valid col_change
    if (col_change > 1 || col_change < -1)
    {
        return false;
    }

    // direction exists
    if (col_change == 0 && row_change == 0)
    {
        return false;
    }

    // passive destinations on the board
    if (!onBoard(p.row+row_change*magnitude, p.column+col_change*magnitude))
    {
        return false;
    }

    // passive has no obstacles
    for (int i = 1; i <= magnitude; ++i)
    {
        // passive move has no obstacles
        if (getField(p.board, p.row+(row_change*i), p.column+(col_change*i)) != EMPTY)
        {
            return false;
        }
    }
    return true;
}

// checks if agressive piece is legal assuming passive piece and vector are legal
bool GameState::isLegalAgressive(Move move) const
{
    // Selected fields are on legal boards
    // passive piece is on homeboard, agressive is on opposite board
    if (move.p.board%2 == move.a.board%2)
    {
        return false;
    }

    // player controls the agressive field
    if (_board[move.a.board][move.a.row][move.a.column] != _turn)
    {
        return false;
    }

    // if board is correct, return if move is valid on the board
    return isLegalAgressive(move.a, move.row_change, move.col_change, move.magnitude);
}

// checks if agressive piece is legal assuming passive piece and vector are legal
bool GameState::isLegalAgressive(Coordinate a, int row_change, int col_change, int magnitude) const
{
    // agressive piece is on a legal field
    if (a.board < 0 || a.board > 3 || !onBoard(a.row, a.column))
    {
        return false;
    }

    // agressive destinations on the board
    if (!onBoard(a.row+row_change*magnitude, a.column+col_change*magnitude))
    {
        return false;
    }

    // move has no obstacles
    int hasPush = 0;
    for (int i = 1; i <= magnitude+hasPush; ++i)
    {
        // agressive move has no obstacles
        if (i < magnitude+1 || onBoard(a.row+row_change*i, a.column+col_change*i))
        {
            Color agressiveTarget = getField(a.board, a.row+(row_change*i), a.column+(col_change*i));
            if (agressiveTarget == _turn) // can not push own piece
            {
                return false;
            }
            else if (agressiveTarget != EMPTY)
            {
                if (++hasPush > 1) // can not push nore than one piece
                {
                    return false;
                }
            }
        }
    }
    return true;
}

// gets all possible moves of the given color
QVector<Move> GameState::getMoves(Color color) const
{
    QVector<Move> ret; // return vector
    QVector<Coordinate> coords[4]; // available pieces

    if (color == EMPTY) // only BLACK or WHITE has moves
    {
        return ret;
    }

    // get own pieces from all boards
    for (int i = 0; i < 4; ++i)
    {
        coords[i] = getPieces(i, color);
    }

    // find current homeboards
    int home_id, opponent_id;
    if (color == WHITE)
    {
        home_id = 2;
        opponent_id = 0;
    }
    else
    {
        home_id = 0;
        opponent_id = 2;
    }

    // add moves from board pairs
    ret += getMovesFromBoards(coords[home_id], coords[opponent_id+1]);
    ret += getMovesFromBoards(coords[home_id+1], coords[opponent_id]);

    // agressives pushing, non pushing are already included as passives
    ret += getAgressiveMovesFromBoards(coords[home_id+1], coords[home_id]);

    ret += getMovesFromBoards(coords[home_id], coords[home_id+1]);

    return ret;
}

// get passive pieces that are part of a legal move
QVector<Coordinate> GameState::getPassivePieces(int board_index) const
{
    QVector<Coordinate> ret; // the vector we return

    if (!isHomeBoard(_turn, board_index)) // passive pieces can only be found on homeboards
    {
        return ret;
    }

    QVector<Move> moves = getAllMoves(); // even redundant moves are important

    for (Move move : moves)
    {
        if (move.p.board == board_index && ret.indexOf(move.p) == -1) // right board, not in return yet
        {
            ret.push_back(move.p);
        }
    }

    return ret;
}

// get passive destinations of passive piece that are part of a legal move
QVector<Coordinate> GameState::getDestinations(int board_index, Coordinate passive) const
{
    QVector<Coordinate> ret;

    if (passive.board != board_index) // if passive is not on board, the destination can not be either
    {
        return ret;
    }

    QVector<Move> moves = getAllMoves(); // even redundant moves are important

    for (Move move : moves)
    {
        if (move.p == passive) // check only those that match passive piece
        {
            // create destination coordinate
            Coordinate destination(passive.board, passive.row+move.magnitude*move.row_change, passive.column+move.magnitude*move.col_change);

            if (ret.indexOf(destination) == -1) // add to return vector if not inside yet
            {
                ret.push_back(destination);
            }
        }
    }

    return ret;
}

// get agressive pieces with given passive and vector that are part of a legal move
QVector<Coordinate> GameState::getAgressivePieces(int board_index, Coordinate passive, int row_change, int col_change, int magnitude) const
{
    QVector<Coordinate> ret;

    if (passive.board % 2 == board_index % 2) // if board is on the same side as the passive move, agressive is not possible
    {
        return ret;
    }

    QVector<Move> moves = getAllMoves(); // even redundant moves are important

    for (Move move : moves)
    {
        if (move.p == passive && row_change == move.row_change && col_change == move.col_change && magnitude == move.magnitude)
        {
            if (move.a.board == board_index && ret.indexOf(move.a) == -1) // add only if not inside yet
            {
                ret.push_back(move.a);
            }
        }
    }
    return ret;
}

// returns a new GameState with a move applied to it
GameState* GameState::getApplied(Move move)
{
    GameState *ret = new GameState();
    ret->setState(this);
    ret->applyMove(move);

    return ret;
}

// apply a move to the current board, return reverse data
ReverseData GameState::applyMove(Move move)
{
    // illegal moves throw an exception
    if (!isLegalMove(move))
    {
        QScopedPointer<ShobuException> exept_ptr(new ShobuException());
        exept_ptr->setMessage("Illegal move");
        exept_ptr->raise();
    }

    // Passive move
    setField(move.p.board, move.p.row+move.row_change*move.magnitude, move.p.column+move.col_change*move.magnitude, _turn);
    setField(move.p.board, move.p.row, move.p.column, EMPTY);

    ReverseData reverse;
    reverse.has_push = false;
    reverse.on_board = false;

    // Agressive move
    //push opposing piece
    for (int i = 1; i <= move.magnitude; ++i)
    {
        Color tempField = getField(move.a.board, move.a.row+move.row_change*i, move.a.column+move.col_change*i);
        if ( tempField == getOpponent())
        {
            reverse.has_push = true;
            reverse.pushed_from = Coordinate(move.a.board, move.a.row+move.row_change*i, move.a.column+move.col_change*i);
            setField(move.a.board, move.a.row+move.row_change*i, move.a.column+move.col_change*i, EMPTY);
        }
    }

    // if piece was pushed to board, place it back
    if (reverse.has_push && onBoard(move.a.row+move.row_change*(move.magnitude+1), move.a.column+move.col_change*(move.magnitude+1)))
    {
        setField(move.a.board, move.a.row+move.row_change*(move.magnitude+1), move.a.column+move.col_change*(move.magnitude+1), getOpponent());

        reverse.on_board = true;
        reverse.pushed_to = Coordinate(move.a.board, move.a.row+move.row_change*(move.magnitude+1), move.a.column+move.col_change*(move.magnitude+1));
    }

    // place own piece
    setField(move.a.board, move.a.row+move.row_change*move.magnitude, move.a.column+move.col_change*move.magnitude, _turn);
    setField(move.a.board, move.a.row, move.a.column, EMPTY);

    endTurn();
    return reverse;
}

// reverse a previous move based on given data
void GameState::reverseMove(Move move, ReverseData data)
{
    endTurn(); // get back the original color as turn

    // reset passives
    setField(move.p.board, move.p.row+move.row_change*move.magnitude, move.p.column+move.col_change*move.magnitude, EMPTY);
    setField(move.p.board, move.p.row, move.p.column, _turn);

    // reset agressives
    setField(move.a.board, move.a.row+move.row_change*move.magnitude, move.a.column+move.col_change*move.magnitude, EMPTY);
    setField(move.a.board, move.a.row, move.a.column, _turn);

    // reset pushed
    if (data.has_push)
    {
        setField(data.pushed_from.board, data.pushed_from.row, data.pushed_from.column, getOpponent());
        if (data.on_board)
        {
            setField(data.pushed_to.board, data.pushed_to.row, data.pushed_to.column, EMPTY);
        }
    }
}

// PRIVATE

// checks if the given coordinates can be on a board
bool GameState::onBoard(int x, int y) const
{
    return 0 <= x && x < 4 && 0 <= y && y < 4;
}

// Step finder functions
// get pieces on the given board of the given color
QVector<Coordinate> GameState::getPieces(int board_id, Color color) const
{
    QVector<Coordinate> ret; // return verctor

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            if (_board[board_id][i][j] == color) // check all fields of all boards
            {
                // if color matches, add to return vector
                ret.push_back(Coordinate(board_id,i,j));
            }
        }
    }

    return ret;
}

// get pieces that can make the given move as passive
QVector<Coordinate> GameState::getPassives(QVector<Coordinate> coords, int row_change, int col_change, int magnitude) const
{
    QVector<Coordinate> ret; // return vector

    for (Coordinate &coord : coords) // check all coordinates
    {
        if (isLegalVector(coord, row_change, col_change, magnitude))
        {
            ret.push_back(coord); // if coordinate can make a passive move with the given vector, add to return
        }
    }
    return ret;
}

// get pieces that can make the given move as agressive (or passive)
QVector<Coordinate> GameState::getAgressives(QVector<Coordinate> coords, int row_change, int col_change, int magnitude) const
{
    QVector<Coordinate> ret; // return vector

    for (Coordinate &coord : coords) // check all coordinates
    {
        if (isLegalAgressive(coord, row_change, col_change, magnitude))
        {
            ret.push_back(coord); // if the piece can make the given move as anagressive move, add to return
        }
    }
    return ret;
}

// get pieces that can make the given move as agressive, but not passive
QVector<Coordinate> GameState::getNonPassiveAgressives(QVector<Coordinate> coords, int row_change, int col_change, int magnitude) const
{
    QVector<Coordinate> ret; // return vector

    for (Coordinate &coord : coords) // check all coordinates
    {
        if (isLegalAgressive(coord, row_change, col_change, magnitude) && !isLegalVector(coord, row_change, col_change, magnitude))
        {
            ret.push_back(coord); // if the piece can push an enemy piece with the given move, add to return
        }
    }
    return ret;
}

// get all legal moves from the two boards assuming boards are legal
QVector<Move> GameState::getMovesFromBoards(QVector<Coordinate> p, QVector<Coordinate> a) const
{
    QVector<Move> ret;

    if (p.isEmpty() || a.isEmpty()) // no moves without pieces
    {
        return ret;
    }

    for (int row_change = -1; row_change <= 1; ++row_change)
    {
        for (int col_change = -1; col_change <= 1; ++col_change)
        {
            if (col_change != 0 || row_change != 0) // only real moves
            {
                QVector<Coordinate> passives = p;
                QVector<Coordinate> agressives = a;
                for (int magnitude = 1; magnitude <= 2; ++magnitude) // check all move vectors
                {
                    // magnitude of 2 is only possible if magnitude of 1 already worked

                    // get pieces that can use the current move vector as passive
                    passives = getPassives(passives, row_change, col_change, magnitude);

                    // get pieces that can use the current move vector as agressive
                    agressives = getAgressives(agressives, row_change, col_change, magnitude);

                    for (Coordinate &passive : passives)
                    {
                        for (Coordinate &agressive : agressives) // add all possible combinations
                        {
                            ret.push_back(Move(passive, agressive, row_change, col_change, magnitude));
                        }
                    }
                }
            }
        }
    }
    return ret;
}

// get all legal agressive moves from the two boards that are not passive assuming boards are legal
QVector<Move> GameState::getAgressiveMovesFromBoards(QVector<Coordinate> p, QVector<Coordinate> a) const
{
    QVector<Move> ret;

    if (p.isEmpty() || a.isEmpty()) // no moves without pieces
    {
        return ret;
    }

    for (int row_change = -1; row_change <= 1; ++row_change)
    {
        for (int col_change = -1; col_change <= 1; ++col_change)
        {
            if (col_change != 0 || row_change != 0) // only real moves
            {
                for (int magnitude = 1; magnitude <= 2; ++magnitude) // check all move vectors
                {
                    // get pieces that can use the current move vector as passive
                    QVector<Coordinate> passives = getPassives(p, row_change, col_change, magnitude);

                    // get pieces that can use the current move vector as agressive, but not as passive
                    QVector<Coordinate> agressives = getNonPassiveAgressives(a, row_change, col_change, magnitude);

                    for (Coordinate &passive : passives)
                    {
                        for (Coordinate &agressive : agressives) // add all possible combinations
                        {
                            ret.push_back(Move(passive, agressive, row_change, col_change, magnitude));
                        }
                    }
                }
            }
        }
    }
    return ret;
}

// gets all possible moves, even redundant ones
QVector<Move> GameState::getAllMoves() const
{
    QVector<Move> ret; // return vector
    QVector<Coordinate> coords[4]; // available pieces

    // get own pieces from all boards
    for (int i = 0; i < 4; ++i)
    {
        coords[i] = getPieces(i, _turn);

    }

    // find current homeboards
    int home_id, opponent_id;
    if (_turn == WHITE)
    {
        home_id = 2;
        opponent_id = 0;
    }
    else
    {
        home_id = 0;
        opponent_id = 2;
    }

    // add moves from board pairs
    ret += getMovesFromBoards(coords[home_id],   coords[opponent_id+1]);
    ret += getMovesFromBoards(coords[home_id+1], coords[opponent_id]);
    ret += getMovesFromBoards(coords[home_id+1], coords[home_id]);
    ret += getMovesFromBoards(coords[home_id],   coords[home_id+1]);

    return ret;
}
