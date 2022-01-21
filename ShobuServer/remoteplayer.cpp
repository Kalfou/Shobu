#include "remoteplayer.h"

#include "gameutils.h"
#include "onlinegame.h"
#include "shobuserver.h"
#include "gamestate.h"

#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDataStream>

enum NetworkEvents
{
    CREATE = 0, // create a new game when received
    JOIN   = 1, // connect player to game when received, notify client of starting game when sent
    LIST   = 2, // send list of all available games
    MOVE   = 3, // send or receive moves
    DRAW   = 4, // send or receive draw offer
    WON    = 5, // send victor to client
    ERROR  = 6  // send error message (client will disconnect after this)
};

// PUBLIC

// Constructor
RemotePlayer::RemotePlayer(ShobuServer *server, QTcpSocket *socket, QObject *parent) : QObject(parent),  _server(server), _sock(socket)
{
    _in_game = false;
    QObject::connect(_sock, &QTcpSocket::readyRead, this, &RemotePlayer::onReadyRead);
    QObject::connect(_sock, &QTcpSocket::disconnected, this, &RemotePlayer::onDisconnect);
}

// receive game from server
void RemotePlayer::setGame(OnlineGame *game)
{
    _game = game;
    _side = _game->getSettings().color == WHITE ? BLACK : WHITE; // joiners get the opposite color
}

// notify player of starting game
void RemotePlayer::startGame()
{
    QJsonObject json;
    json["event"] = JOIN;
    json["name"] = _game->getSettings().name;
    json["color"] = _side;                             // side is determined when player created or joined a game
    json["has_time"] = _game->getSettings().has_time;
    if (_game->getSettings().has_time)
    {
        json["time"] = _game->getSettings().time;
    }

    sendJson(json);
}

// notify player of move of opponent
void RemotePlayer::sendMove(Move move)
{
    QJsonObject json;

    json["event"] = MOVE;

    json["p_board"]  = move.p.board;
    json["p_row"]    = move.p.row;
    json["p_column"] = move.p.column;

    json["row_change"] = move.row_change;
    json["col_change"] = move.col_change;
    json["magnitude"] = move.magnitude;

    json["a_board"]  = move.a.board;
    json["a_row"]    = move.a.row;
    json["a_column"] = move.a.column;

    sendJson(json);
}

// notify player of draw offer of opponent
void RemotePlayer::sendDrawOffer()
{
    QJsonObject json;

    json["event"] = DRAW;

    sendJson(json);
}

// notify player of winner
void RemotePlayer::sendVictor(Color color)
{
    QJsonObject json;

    json["event"] = WON;

    json["color"] = color;

    sendJson(json);
}

// sends an error message to the client
void RemotePlayer::sendError(QString message)
{
    QJsonObject json;

    json["event"]   = ERROR;
    json["message"] = message;

    sendJson(json);
}

// PRIVATE

// delete player, end game if necessary
void RemotePlayer::onDisconnect()
{
    if (!_in_game)
    {
        deleteLater();
    }
    else
    {
        _game->playerLeft(_side);
    }
}

//form json out of message; forward it to readJson()
void RemotePlayer::onReadyRead()
{
    QByteArray json_data;
    QDataStream socket_stream(_sock);
    socket_stream.setVersion(QDataStream::Qt_5_9);

    //read till we get the valid json
    bool loop = true;
    while (loop)
    {
        socket_stream.startTransaction();
        socket_stream >> json_data;
        if (socket_stream.commitTransaction())
        {
            QJsonParseError parse_error;
            const QJsonDocument json = QJsonDocument::fromJson(json_data, &parse_error);

            // check if json is valid
            if (parse_error.error == QJsonParseError::NoError)
            {
                if (json.isObject())
                {
                    readJson(json.object());
                }
            }
        }
        else // stop when json ended
        {
            loop = false;
        }
    }

}

// sends the message to the server
void RemotePlayer::sendJson(const QJsonObject &json)
{
    // use QJsonDocument::Compact to save bandwidth
    const QByteArray json_data = QJsonDocument(json).toJson(QJsonDocument::Compact);

    // send
    QDataStream socket_stream(_sock);
    socket_stream.setVersion(QDataStream::Qt_5_9);
    socket_stream << json_data;
}

// receive and handle message from player
void RemotePlayer::readJson(const QJsonObject &json)
{
    // get type of event received
    const QJsonValue event_type = json.value(QLatin1String("event"));

    if (event_type.isNull() || !event_type.isDouble())
    {
        sendError();
        return;
    }

    // handle event based on type
    switch (static_cast<NetworkEvents>(event_type.toInt()))
    {
    case CREATE:
        createGame(json);
        break;
    case JOIN:
        joinGame(json);
        break;
    case LIST:
        listGames();
        break;
    case MOVE:
        makeMove(json);
        break;
    case DRAW:
        _game->offerDraw(_side);
        break;
    default:
        sendError("An unknown server error occured");
        break;
    }
}

// tries to create a game asked in the json
void RemotePlayer::createGame(const QJsonObject &json)
{
    GameSettings settings;
    settings.style = NETWORK;

    // set name
    QJsonValue json_property = json.value(QLatin1String("name"));

    if (json_property.isNull() || !json_property.isString())
    {
        sendError();
        return;
    }

    settings.name = json_property.toString();

    //set color
    json_property = json.value(QLatin1String("color"));

    if (json_property.isNull() || !json_property.isDouble())
    {
        sendError();
        return;
    }

    settings.color = static_cast<Color>(json_property.toInt());
    if (settings.color != WHITE && settings.color != BLACK)
    {
        sendError();
        return;
    }

    // set time
    json_property = json.value(QLatin1String("has_time"));

    if (json_property.isNull() || !json_property.isBool())
    {
        sendError();
        return;
    }

    settings.has_time = json_property.toBool();

    // add time values if it has time
    if (settings.has_time)
    {
        json_property = json.value(QLatin1String("time"));

        if (json_property.isNull() || !json_property.isDouble())
        {
            sendError();
            return;
        }
        settings.time = json_property.toInt();
    }
    _game = new OnlineGame(this, settings, this);
    if (!_server->addGame(_game))
    {
        _game->deleteLater();
        sendError("Name is already taken");
        return;
    }

    setParent(_game);
    _side = settings.color;
    _in_game = true; // if game is created, we are in game
}

// returns available games to client
void RemotePlayer::listGames()
{
    QJsonObject json;
    QVector<OnlineGame*> games = _server->getGames();
    QJsonArray array;

    for (OnlineGame* game : games)
    {
        if (!game->isStarted())
        {
            QJsonObject entry;
            entry["name"] = game->getSettings().name;
            entry["color"] = game->getSettings().color == WHITE ? BLACK : WHITE; // opponent gets the other color
            entry["has_time"] = game->getSettings().has_time;
            if (game->getSettings().has_time)
            {
                entry["time"] = game->getSettings().time;
            }
            array.append(entry);
        }
    }

    json["event"] = LIST;
    json["games"] = array;

    sendJson(json);
}

// attempts to jion a game based on the json
void RemotePlayer::joinGame(const QJsonObject &json)
{
    QJsonValue json_property = json.value(QLatin1String("name"));

    if (json_property.isNull() || !json_property.isString())
    {
        sendError();
        return;
    }

    QString name = json_property.toString();

    _in_game = _server->joinGame(name, this); // if join successful, we are in game

    if (!_in_game)
    {
        sendError("Game no longer available");
    }
    else
    {
        setParent(_game);
    }
}

// makes a move based on the json
void RemotePlayer::makeMove(const QJsonObject &json)
{
    Move move;

    // passive move
    QJsonValue json_property = json.value(QLatin1String("p_board"));
    if (json_property.isNull() || !json_property.isDouble())
    {
        sendError();
        return;
    }
    move.p.board = json_property.toInt();

    json_property = json.value(QLatin1String("p_row"));
    if (json_property.isNull() || !json_property.isDouble())
    {
        sendError();
        return;
    }
    move.p.row = json_property.toInt();

    json_property = json.value(QLatin1String("p_column"));
    if (json_property.isNull() || !json_property.isDouble())
    {
        sendError();
        return;
    }
    move.p.column = json_property.toInt();

    // vector
    json_property = json.value(QLatin1String("row_change"));
    if (json_property.isNull() || !json_property.isDouble())
    {
        sendError();
        return;
    }
    move.row_change = json_property.toInt();

    json_property = json.value(QLatin1String("col_change"));
    if (json_property.isNull() || !json_property.isDouble())
    {
        sendError();
        return;
    }
    move.col_change = json_property.toInt();

    json_property = json.value(QLatin1String("magnitude"));
    if (json_property.isNull() || !json_property.isDouble())
    {
        sendError();
        return;
    }
    move.magnitude = json_property.toInt();

    // agressive move
    json_property = json.value(QLatin1String("a_board"));
    if (json_property.isNull() || !json_property.isDouble())
    {
        sendError();
        return;
    }
    move.a.board = json_property.toInt();

    json_property = json.value(QLatin1String("a_row"));
    if (json_property.isNull() || !json_property.isDouble())
    {
        sendError();
        return;
    }
    move.a.row = json_property.toInt();

    json_property = json.value(QLatin1String("a_column"));
    if (json_property.isNull() || !json_property.isDouble())
    {
        sendError();
        return;
    }
    move.a.column = json_property.toInt();

    _game->makeMove(move, _side);
}
