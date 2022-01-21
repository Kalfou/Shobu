#include "shobuclient.h"

#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDataStream>

#include "gamestate.h"
#include "shobuplayer.h"

enum ClientValues
{
    PORT = 1234,             // the port where we will connect
    CONNECT_TIMEOUT = 3000   // the time we wait for connecting
};

enum NetworkEvenets // event codes
{
    CREATE = 0, // creation of a game
    JOIN   = 1, // try to join to a game when sent, game starts when received
    LIST   = 2, // ask for available games when sent, get list when received
    MOVE   = 3, // send our move or receive opponents move
    DRAW   = 4, // offer draw or receive draw offer
    WON    = 5, // receive the victor
    ERROR  = 6  // server no longer works as intended
};

// PUBLIC

// Constructor
ShobuClient::ShobuClient(QObject *parent) : QObject(parent)
{
    _sock = new QTcpSocket(this);

    QObject::connect(_sock, &QTcpSocket::readyRead,    this, &ShobuClient::onReadyRead);
    QObject::connect(_sock, &QTcpSocket::disconnected, this, &ShobuClient::onDisconnected);
}

// connect to server with given hostname, return true on success
bool ShobuClient::connectToServer(QString hostname)
{
    _sock->connectToHost(hostname, PORT);
    _connected = _sock->waitForConnected(CONNECT_TIMEOUT);
    return _connected;
}

// disconnect from server
void ShobuClient::disconnectFromServer()
{
    _connected = false; // the disconnect is natural, no need for error
    _sock->disconnectFromHost();
}

// set the player on the client side
void ShobuClient::setPlayer(ShobuPlayer *player)
{
    _player = player;
}

// create a message to request a new game from the server
void ShobuClient::createGame(GameSettings settings)
{
    QJsonObject json;

    json["event"]    = CREATE;
    json["name"]     = settings.name;
    json["style"]    = NETWORK;
    json["color"]    = settings.color;
    json["has_time"] = settings.has_time;

    if (settings.has_time)
    {
        json["time"] = settings.time;
    }

    sendJson(json);
}

// creates a message to ask for available games
void ShobuClient::askForGames()
{
    QJsonObject json;

    json["event"] = LIST;

    sendJson(json);
}

// creates a message to join a game
void ShobuClient::joinGame(QString name)
{
    QJsonObject json;

    json["event"] = JOIN;
    json["name"]  = name;

    sendJson(json);
}

// creates a message notifying the server of a move
void ShobuClient::sendMove(Move move)
{
    QJsonObject json;

    json["event"] = MOVE;

    json["p_board"]    = move.p.board;
    json["p_row"]      = move.p.row;
    json["p_column"]   = move.p.column;

    json["row_change"] = move.row_change;
    json["col_change"] = move.col_change;
    json["magnitude"]  = move.magnitude;

    json["a_board"]    = move.a.board;
    json["a_row"]      = move.a.row;
    json["a_column"]   = move.a.column;

    sendJson(json);
}

// creates a message to offer a draw
void ShobuClient::offerDraw()
{
    QJsonObject json;

    json["event"] = DRAW;

    sendJson(json);
}

// PRIVATE

// notify model of losing the connection, if it did not terminate it
void ShobuClient::onDisconnected()
{
    if(_connected)
    {
        emit serverMessage("Connection lost to server");
    }
}

// read a json from the server
void ShobuClient::onReadyRead()
{
    QByteArray json_data;
    QDataStream socket_stream(_sock);
    socket_stream.setVersion(QDataStream::Qt_5_9);

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
                    readJson(json.object()); // if valid vector is reveived, we can process it
                }
            }
        }
        else // stop when json ended
        {
            loop = false;
        }
    }
}

// process a json received from the server
void ShobuClient::readJson(const QJsonObject &json)
{
    const QJsonValue event_type = json.value(QLatin1String("event")); // get event type

    if (event_type.isNull() || !event_type.isDouble())
    {
        emit serverMessage("Unknown server error occured");
        return;
    }

    switch (static_cast<NetworkEvenets>(event_type.toInt()))
    {
    case CREATE:
        break;
    case JOIN:
        startGame(json);  // if both players joined, we can start
        break;
    case LIST:
        getGameList(json);
        break;
    case MOVE:
        getMove(json);
        break;
    case DRAW:
        emit drawOffer();
        break;
    case WON:
        getVictor(json);
        break;
    case ERROR:
        getError(json);
        break;
    default:
        emit serverMessage("Unknown server error occured");
        break;
    }
}

// sends the message to the server
void ShobuClient::sendJson(const QJsonObject &json)
{
    // use QJsonDocument::Compact to save bandwidth
    const QByteArray json_data = QJsonDocument(json).toJson(QJsonDocument::Compact);

    // send
    QDataStream socket_stream(_sock);
    socket_stream.setVersion(QDataStream::Qt_5_9);
    socket_stream << json_data;
}

// get the game list sent in json, forward it to model in signal
void ShobuClient::getGameList(const QJsonObject &json)
{
    QVector<GameSettings> game_list; // vector to return with a signal

    // get game list
    QJsonValue raw_games = json.value(QLatin1String("games"));

    if (raw_games.isNull() || !raw_games.isArray())
    {
        return;
    }
    QJsonArray games = raw_games.toArray();

    for (int i = 0; i < games.size(); ++i) // process each game in list
    {
        if (games[i].isObject())
        {
            bool valid = true;
            GameSettings settings;
            settings.style = NETWORK;

            QJsonObject game = games[i].toObject();

            // set name
            QJsonValue game_property = game.value(QLatin1String("name"));
            if (valid = valid && !game_property.isNull() && game_property.isString(); valid)
            {
                settings.name = game_property.toString();
            }

            // set color
            game_property = game.value(QLatin1String("color"));
            if (valid = valid && !game_property.isNull() && game_property.isDouble(); valid)
            {
                // color must be black or white
                if (Color color = static_cast<Color>(game_property.toInt()); color == WHITE || color == BLACK)
                {
                    settings.color = color;
                }
                else
                {
                    valid = false;
                }
            }

            // set has_time
            game_property = game.value(QLatin1String("has_time"));
            if (valid = valid && !game_property.isNull() && game_property.isBool(); valid)
            {
                settings.has_time = game_property.toBool();
            }

            // if game has time, set time
            if (valid && settings.has_time)
            {
                game_property = game.value(QLatin1String("time"));
                if (valid = valid && !game_property.isNull() && game_property.isDouble(); valid)
                {
                    settings.time = game_property.toInt();
                    settings.times[WHITE] = settings.time;
                    settings.times[BLACK] = settings.time;
                }
            }

            // if settings are valid, add to list
            if (valid)
            {
                game_list.push_back(settings);
            }
        }
    }
    // send games to model
    emit gamesArrived(game_list);
}

// start a new game with the settings received in a json
void ShobuClient::startGame(const QJsonObject &json)
{
    bool valid = true;
    GameSettings settings;
    settings.style = NETWORK;

    // set name
    QJsonValue game_property = json.value(QLatin1String("name"));
    if (valid = valid && !game_property.isNull() && game_property.isString(); valid)
    {
        settings.name = game_property.toString();
    }

    // set color
    game_property = json.value(QLatin1String("color"));
    if (valid = valid && !game_property.isNull() && game_property.isDouble(); valid)
    {
        // color must be black or white
        if (Color color = static_cast<Color>(game_property.toInt()); color == WHITE || color == BLACK)
        {
            settings.color = color;
        }
        else
        {
            valid = false;
        }
    }

    // set has_time
    game_property = json.value(QLatin1String("has_time"));
    if (valid = valid && !game_property.isNull() && game_property.isBool(); valid)
    {
        settings.has_time = game_property.toBool();
    }

    // if game has time, set time
    if (valid && settings.has_time)
    {
        game_property = json.value(QLatin1String("time"));
        if (valid = valid && !game_property.isNull() && game_property.isDouble(); valid)
        {
            settings.time = game_property.toInt();
            settings.times[WHITE] = settings.time;
            settings.times[BLACK] = settings.time;
        }
    }

    // notify model of result
    if (valid)
    {
        emit gameStarted(settings);
    }
    else
    {
        emit serverMessage("Could not connect to game");
    }
}

// get opponents move from json
void ShobuClient::getMove(const QJsonObject &json)
{
    Move move;

    // passive move
    QJsonValue json_property = json.value(QLatin1String("p_board"));

    if (json_property.isNull() || !json_property.isDouble())
    {
        emit serverMessage("Connection broken with server");
        return;
    }
    move.p.board = json_property.toInt();


    json_property = json.value(QLatin1String("p_row"));

    if (json_property.isNull() || !json_property.isDouble())
    {
        emit serverMessage("Connection broken with server");
        return;
    }
    move.p.row = json_property.toInt();


    json_property = json.value(QLatin1String("p_column"));

    if (json_property.isNull() || !json_property.isDouble())
    {
        emit serverMessage("Connection broken with server");
        return;
    }
    move.p.column = json_property.toInt();


    // vector
    json_property = json.value(QLatin1String("row_change"));

    if (json_property.isNull() || !json_property.isDouble())
    {
        emit serverMessage("Connection broken with server");
        return;
    }
    move.row_change = json_property.toInt();


    json_property = json.value(QLatin1String("col_change"));

    if (json_property.isNull() || !json_property.isDouble())
    {
        emit serverMessage("Connection broken with server");
        return;
    }
    move.col_change = json_property.toInt();


    json_property = json.value(QLatin1String("magnitude"));

    if (json_property.isNull() || !json_property.isDouble())
    {
        emit serverMessage("Connection broken with server");
        return;
    }
    move.magnitude = json_property.toInt();

    // agressive move
    json_property = json.value(QLatin1String("a_board"));

    if (json_property.isNull() || !json_property.isDouble())
    {
        emit serverMessage("Connection broken with server");
        return;
    }
    move.a.board = json_property.toInt();


    json_property = json.value(QLatin1String("a_row"));

    if (json_property.isNull() || !json_property.isDouble())
    {
        emit serverMessage("Connection broken with server");
        return;
    }
    move.a.row = json_property.toInt();


    json_property = json.value(QLatin1String("a_column"));

    if (json_property.isNull() || !json_property.isDouble())
    {
        emit serverMessage("Connection broken with server");
        return;
    }
    move.a.column = json_property.toInt();

    // give move to our player
    _player->makeMove(move);
}

// get victor from json
void ShobuClient::getVictor(const QJsonObject &json)
{
    QJsonValue color = json.value(QLatin1String("color"));

    if (color.isNull() || !color.isDouble())
    {
        emit serverMessage("Connection broken with server");
        return;
    }
    switch (static_cast<Color>(color.toInt()))
    {
    case WHITE:
        emit serverMessage("White won the game!");
        break;
    case BLACK:
        emit serverMessage("Black won the game");
        break;
    case EMPTY:
        emit serverMessage("The game ended with a draw.");
        break;
    default:
        emit serverMessage("Connection broken with server");
        break;
    }
}

// get server error message from json
void ShobuClient::getError(const QJsonObject &json)
{
    QJsonValue message = json.value(QLatin1String("message"));
    if (!message.isNull() && message.isString())
    {
        emit serverMessage(message.toString());
    }
    else
    {
        emit serverMessage("Unknown server error occured");
    }
}
