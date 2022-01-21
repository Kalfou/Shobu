#ifndef SHOBUSERVER_H
#define SHOBUSERVER_H

#include <QObject>

#include <QTcpServer>
#include <QDebug>

class QTcpSocket;
class QTimer;

class RemotePlayer;
class OnlineGame;

class ShobuServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit ShobuServer(QObject *parent = nullptr);

    QVector<OnlineGame*> getGames() const {return _active_games;}

    bool addGame(OnlineGame *game);
    bool joinGame(QString name, RemotePlayer *joiner);

public slots:
    void onNewConnection();
    void onGameEnded(OnlineGame *game);
    void onTimeout();

private:
    QTimer *_timer;

    QVector<OnlineGame*> _active_games;

    bool isNameFree(QString name) const;

};

#endif // SHOBUSERVER_H
