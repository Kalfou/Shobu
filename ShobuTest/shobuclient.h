#ifndef SHOBUCLIENT_H
#define SHOBUCLIENT_H

#include <QObject>

#include "gameutils.h"
#include "gamestate.h"

class QTcpSocket;
class ShobuPlayer;

class ShobuClient : public QObject
{
    Q_OBJECT
public:
    explicit ShobuClient(QObject *parent = nullptr);

    bool connectToServer(QString hostname);
    void disconnectFromServer();
    void setPlayer(ShobuPlayer *player);

    // send to server
    void createGame(GameSettings settings);
    void askForGames();
    void joinGame(QString name);
    void sendMove(Move move);
    void offerDraw();

private:
    ShobuPlayer *_player;
    QTcpSocket *_sock;
    bool _connected; //

    void onDisconnected();

    // message sending and receiving
    void onReadyRead();
    void readJson(const QJsonObject &json);
    void sendJson(const QJsonObject &json);

    //receive from server
    void getGameList(const QJsonObject &json);
    void startGame(const QJsonObject &json);
    void getMove(const QJsonObject &json);
    void getVictor(const QJsonObject &json);
    void getError(const QJsonObject &json);

signals:
    void gamesArrived(QVector<GameSettings>);
    void gameStarted(GameSettings);
    void drawOffer();
    void serverMessage(QString);
};

#endif // SHOBUCLIENT_H
