#ifndef SHOBUPERSISTENCE_H
#define SHOBUPERSISTENCE_H

#include <QObject>
#include <QVector>

#include "gameutils.h"

class GameState;

class ShobuPersistence : public QObject
{
    Q_OBJECT
public:
    ShobuPersistence(GameState *state, QObject *parent = nullptr);

    void initialize();

    bool saveGame(QString filename, GameSettings settings);
    bool loadGame(const QString &filename, GameSettings &settings);

    static QStringList getSaves();
    static bool deleteSave(QString save);

    bool undoStep(int backstep);
    bool redoStep(int step);
    void saveState();

    bool hasBackState(int backstep) const {return current>=backstep;}
    bool hasForwardState(int step) const {return current+step<=top;}

    private:
    GameState *_game;
    QVector<GameState*> _states;
    int top, current;

    void clearVector();
};

#endif // SHOBUPERSISTENCE_H
