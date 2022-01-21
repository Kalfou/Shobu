#ifndef SHOBUMODEL_H
#define SHOBUMODEL_H

#include <QObject>

#include "gamestate.h"
#include "shobupersistence.h"
#include "gameutils.h"
#include "shobuplayer.h"

class QTimer;
class ShobuClient;
class ShobuPersistence;

class ShobuModel : public QObject
{
    Q_OBJECT
public:
    explicit ShobuModel(QObject *parent);

    // Getters
    Color getField(int board, int row, int column) const {return _game->getField(board, row, column);}
    MoveState *getMoveState() const {return _move;}
    ShobuPlayer *getPlayer(Color color) const {return _players[color];}
    ShobuPlayer *getPlayer() const {return getPlayer(_game->getTurn());}
    GameSettings getSettings() const {return _settings;}
    bool getTicking() const {return _ticking;};
    bool getDrawOffer(Color color) const {return _settings.style==NETWORK && _draw_offer[color];}
    bool getDrawOffer() const {return getDrawOffer(_settings.color);}

    // Setter
    void setSettings(GameSettings settings) {_settings = settings;}
    void changeTime() {_ticking = !_ticking;};

    // Game Controllers
    void newGame(GameSettings settings);
    void makeMove();
    void resetMove();
    void endGame();

    void changeSettings(GameSettings settings);

    //Persistence
    bool saveGame(const QString filename);
    bool loadGame(const QString filename);
    bool undoStep();
    bool redoStep();
    bool hasUndo();
    bool hasRedo();

    // Network
    bool connectToServer(QString hostname);
    void disconnectFromServer();
    void createOnlineGame(GameSettings settings);
    void getOnlineGameList();
    void joinOnlineGame(QString name);
    void offerDraw();

private:
    GameState *_game;
    MoveState *_move;
    GameSettings _settings;

    ShobuPlayer *_players[2];
    QTimer *_timer;
    bool _ticking;
    int _tick_count;
    bool _move_ready;

    ShobuPersistence *_persistence;

    ShobuClient *_client;
    bool _draw_offer[2];

    // Game controllers
    void initializeGame();
    int getStepSize();
    void applySettings();
    void applyMove();

    // Network
    void onGameStarted(GameSettings settings);
    void onDrawOffer();

    // Timer
    void tick();

signals:
    // Game
    void gameOver(Color);
    void stepGame();
    void boardChange();

    // Timer
    void timeIsPassing();

    //Network
    void gotOnlineList(QVector<GameSettings>);
    void gotServerMessage(QString);
    void onlineGameStarted();
};

#endif // SHOBUMODEL_H
