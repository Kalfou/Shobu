#ifndef REMOTEPLAYER_H
#define REMOTEPLAYER_H

#include <QObject>

#include "gameutils.h"

class QTcpSocket;
class ShobuServer;
class OnlineGame;
struct Move;

class RemotePlayer : public QObject
{
    Q_OBJECT
public:
    RemotePlayer(ShobuServer *server, QTcpSocket *socket, QObject *parent = nullptr);

    // Setters
    void setGame(OnlineGame *game);

    void startGame();
    void sendMove(Move move);
    void sendDrawOffer();
    void sendVictor(Color color);

    void sendError(QString message = "Connection broken with server");

private:
    ShobuServer *_server;
    QTcpSocket  *_sock;
    OnlineGame   *_game;
    bool _in_game;
    Color _side;

    void onDisconnect();
    void onReadyRead();
    void sendJson(const QJsonObject &json);
    void readJson(const QJsonObject &json);

    void createGame(const QJsonObject &json);
    void listGames();
    void joinGame(const QJsonObject &json);
    void makeMove(const QJsonObject &json);
};

#endif // REMOTEPLAYER_H
