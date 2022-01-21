#ifndef ONLINEGAME_H
#define ONLINEGAME_H

#include <QObject>

#include "gameutils.h"

class GameState;
class RemotePlayer;
struct Move;

class OnlineGame : public QObject
{
    Q_OBJECT
public:
    OnlineGame(RemotePlayer *player, GameSettings settings, QObject *parent = nullptr);

    // Getters
    bool isStarted() const {return _started;}
    GameSettings getSettings() const {return _settings;}

    void joinGame(RemotePlayer *player);
    void makeMove(Move move, Color color);
    void offerDraw(Color color);

    void playerLeft(Color color);

    void tick();

private:
    GameSettings _settings;
    GameState *_game;
    RemotePlayer *_players[2];

    bool _started;
    bool _ended;
    bool _draw_offered[2];

    void gameWon(Color color);

signals:
    void gameEnded(OnlineGame*);
};

#endif // ONLINEGAME_H
