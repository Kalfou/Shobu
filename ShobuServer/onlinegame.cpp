#include "onlinegame.h"

#include "remoteplayer.h"
#include "gamestate.h"

// PUBLIC

// Constructor
OnlineGame::OnlineGame(RemotePlayer *player, GameSettings settings, QObject *parent) : QObject(parent), _settings(settings)
{
    _game = new GameState(this);

    _started             = false;
    _ended               = false;
    _draw_offered[WHITE] = false;
    _draw_offered[BLACK] = false;


    if (_settings.has_time)
    {
        _settings.times[WHITE] = _settings.time;
        _settings.times[BLACK] = _settings.time;
    }

    _players[_settings.color] = player;

    _started = false;
}

// new player takes the free color
void OnlineGame::joinGame(RemotePlayer *player)
{
    _started = true;

    _players[_settings.color == WHITE ? BLACK : WHITE] = player; //joiner gets the opposite color

    _game->initializeGame();

    // notify players of the starting game
    _players[WHITE]->startGame();
    _players[BLACK]->startGame();
}

// handle move of player of given color
void OnlineGame::makeMove(Move move, Color color)
{
    if (_game->getTurn() == color)
    {
        if (_game->isLegalMove(move)) // if everything is well, make move and notify opponent
        {
            _game->applyMove(move);

            // a move automatically refuses draw offers
            _draw_offered[WHITE] = false;
            _draw_offered[BLACK] = false;

            Color opponent = color == WHITE ? BLACK : WHITE;
            _players[opponent]->sendMove(move);
        }
        else // illegal move ends the game
        {
            _players[color]->sendError();
        }
    }
    else // if wrong player manages to make move, the game ends
    {
        _players[color]->sendError();
    }
}

// handle draw request of player with given color
void OnlineGame::offerDraw(Color color)
{
    _draw_offered[color] = true;

    _players[color == WHITE ? BLACK : WHITE]->sendDrawOffer(); // the other player gets notified

    if (_draw_offered[WHITE] && _draw_offered[BLACK]) // if both players want draw, the game ends with draw
    {
        gameWon(EMPTY);
    }
}

// handle if a player leaves mid game
void OnlineGame::playerLeft(Color color)
{
    if (!_ended) // if the game did not end, the opponent won
    {
        _ended = true;
        if (_started)  // if game started, opponent wins
        {
            Color victor = color == WHITE ? BLACK : WHITE;
            _players[victor]->sendVictor(victor);
        }
        emit gameEnded(this);
    }
}

// time passes if necessary
void OnlineGame::tick()
{
    if (_settings.has_time && _started && !_ended) // if game is on, current player loses time
    {
        if (0 >= --_settings.times[_game->getTurn()]) // if time is up, opponent wins the game
        {
            gameWon(_game->getOpponent());
        }
    }
}

// PRIVATE

// notify players of victor and end game
void OnlineGame::gameWon(Color color)
{
    _ended = true;

    _players[WHITE]->sendVictor(color);
    _players[BLACK]->sendVictor(color);

    emit gameEnded(this);
}
