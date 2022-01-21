#include "shobuserver.h"

#include "onlinegame.h"
#include "remoteplayer.h"
#include "shobugame.h"

#include <QTcpSocket>
#include <QTimer>

enum ServerValues
{
    INTERVAL = 1000 // tick interval for the server
};

// PUBLIC

// Constructor, tells if port is successfully connected
ShobuServer::ShobuServer(QObject *parent) : QTcpServer(parent)
{
    QObject::connect(this, SIGNAL(newConnection()), this, SLOT(onNewConnection()));

    if (listen(QHostAddress::Any, 1234))
    {
        qDebug()<<"Connected";
    }
    else
    {
        qDebug()<<"Not connected";
    }

    _timer = new QTimer(this);
    _timer->setInterval(INTERVAL);

    QObject::connect(_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    _timer->start();
}

// tries to create a new game with the given settings, returns true on success
bool ShobuServer::addGame(OnlineGame *game)
{
    if (!isNameFree(game->getSettings().name))
    {
        return false;
    }

    QObject::connect(game, SIGNAL(gameEnded(OnlineGame*)), this, SLOT(onGameEnded(OnlineGame*)));

    game->setParent(this);
    _active_games.push_back(game);
    return true;
}

// join the game with the given name. Return true on success.
bool ShobuServer::joinGame(QString name, RemotePlayer *joiner)
{
    // search for game
    for (int i = 0; i < _active_games.length(); ++i)
    {
        if (_active_games[i]->getSettings().name == name)
        {
            if (!_active_games[i]->isStarted())
            {
                // game found and available
                joiner->setGame(_active_games[i]);
                _active_games[i]->joinGame(joiner);
                return true;
            }
            else // found the game, but it is taken
            {
                return false;
            }
        }
    }
    // did not find the game
    return false;
}

// PUBLIC SLOTS

// creates new OnlinePlayer class for newly cnnected user
void ShobuServer::onNewConnection()
{
    QTcpSocket *new_socket = nextPendingConnection();
    new RemotePlayer(this, new_socket, this);
}

// Deletes a game when it is ended
void ShobuServer::onGameEnded(OnlineGame *game)
{
    _active_games.removeOne(game);

    game->deleteLater();
}

// time passes for active games with timer
void ShobuServer::onTimeout()
{
    for (int i = 0; i < _active_games.length(); ++i)
    {
        _active_games[i]->tick(); // time passes for everyone
    }
}

// PRIVATE

// returns true if name is not taken yet
bool ShobuServer::isNameFree(QString name) const
{
    for (int i = 0; i < _active_games.length(); ++i)
    {
        if (_active_games[i]->getSettings().name == name)
        {
            return false;
        }
    }
    return true;
}
