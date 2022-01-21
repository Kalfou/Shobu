#ifndef SHOBUGAME_H
#define SHOBUGAME_H

#include <QObject>

#include "gameutils.h"

class GameState;
class RemotePlayer;

class ShobuGame : public QObject
{
    Q_OBJECT
public:
    ShobuGame(GameSettings settings, RemotePlayer *creator,  QObject *parent = nullptr);

    // Getters
    bool getInProgress() const {return _in_progress;}

private:
    bool _in_progress;
    GameState *_state;
    GameSettings _settings;
    RemotePlayer *_players[2];

signals:

};

#endif // SHOBUGAME_H
