#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <QObject>
#include <QVector>

#include "gameutils.h"

// Contains the coordinate of a field
struct Coordinate
{
    int board, row, column;

    // Constructors
    Coordinate(): board(0), row(0), column(0){};
    Coordinate(int b, int r, int c): board(b), row(r), column(c){}

    // Operators
    bool operator==(const Coordinate& c) { return board==c.board && row==c.row && column==c.column;}
};

// Contains the pieces and the vector of the move
struct Move
{
    Coordinate p;                 // coordinates of the passive piece
    Coordinate a;                 // coordinates of the agressive piece
    int row_change, col_change;   // change on the given coordinate (-1, 0, 1)
    int magnitude;                // magnitude of the change (1, 2)

    // Constructors
    Move(): p(Coordinate()), a(Coordinate()), row_change(1), col_change(1), magnitude(1) {};
    Move(Coordinate pass, Coordinate agr, int rc, int cc, int m) : p(pass), a(agr), row_change(rc), col_change(cc), magnitude(m) {};
};

// Contains data needed to reverse a move besides the move
struct ReverseData
{
    bool has_push;           // true, if there has been a push
    Coordinate pushed_from;  // if has_pushed is true, this contains the original coordinates of the pushed piece
    bool on_board;           // true, if has_pushed is true and the pushed piece remained on the board
    Coordinate pushed_to;    // if on_board is true, this contains the destination coordinates of the pushed piece
};

class GameState : public QObject
{
    Q_OBJECT
public:
    GameState(QObject *parent = nullptr) : QObject(parent){};

    void initializeGame();

    // Getters
    Color getField(int table, int row, int column) const {return _board[table][row][column];}
    Color getTurn() const {return _turn;}
    static Color getOpponent(Color color) {return color == WHITE ? BLACK : WHITE;}
    Color getOpponent() const {return getOpponent(_turn);}

    static bool isHomeBoard(Color color, int board_id);

    // Setter
    void setState(const GameState *fromState);
    void setField(int table, int row, int column, Color color);
    void setTurn(Color color);

    // Step functions
    void makeMove(Move move);
    void endTurn();
    Color getVictor() const;

    bool hasMoves() const;

    bool isLegalMove(Move move) const;

    // Player needs to verify by steps
    bool isLegalPassive(Coordinate coord) const;
    bool isLegalVector(Coordinate p, int row_change, int col_change, int magnitude) const;
    bool isLegalAgressive(Move move) const;
    bool isLegalAgressive(Coordinate a, int row_change, int col_change, int magnitude) const; // without board check

    // Step finder functions
    QVector<Move> getMoves(Color color) const;
    QVector<Move> getMoves() const {return getMoves(_turn);};

    QVector<Coordinate> getPassivePieces(int board_index) const;
    QVector<Coordinate> getDestinations(int board_index, Coordinate passive) const;
    QVector<Coordinate> getAgressivePieces(int board_index, Coordinate passive, int row_change, int col_change, int magnitude) const;

    GameState* getApplied(Move move);
    ReverseData applyMove(Move move);
    void reverseMove(Move move, ReverseData data);

private:
    Color _board[4][4][4];
    Color _turn;

    bool onBoard(int x, int y) const;

    // Step finder functions
    QVector<Coordinate> getPieces(int board_id, Color color) const;
    QVector<Coordinate> getPassives(QVector<Coordinate> coords, int row_change, int col_change, int magnitude) const;
    QVector<Coordinate> getAgressives(QVector<Coordinate> coords, int row_change, int col_change, int magnitude) const;
    QVector<Coordinate> getNonPassiveAgressives(QVector<Coordinate> coords, int row_change, int col_change, int magnitude) const;
    QVector<Move> getMovesFromBoards(QVector<Coordinate> p, QVector<Coordinate> a) const;
    QVector<Move> getAgressiveMovesFromBoards(QVector<Coordinate> p, QVector<Coordinate> a) const;

    QVector<Move> getAllMoves() const;
};

struct MoveState // the players and the game communicate through this
{
    Move move;
    GameState *game;
    bool passive_set;
    bool vector_set;
};

#endif // GAMESTATE_H
