#include "shobugame.h"

#include "gamestate.h"
#include "remoteplayer.h"

ShobuGame::ShobuGame(GameSettings settings, RemotePlayer *creator,  QObject *parent) : QObject(parent), _settings(settings)
{
    _in_progress = false;
    _state = new GameState(this);
    _players[settings.color] = creator;
}
