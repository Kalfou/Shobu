#ifndef GAMEUTILS_H
#define GAMEUTILS_H

#include <QString>

enum GameStyle
{
    SOLO = 0,
    HOTSEAT = 1,
    NETWORK = 2
};

enum Difficulty
{
    EASY = 0,
    MEDIUM = 1,
    HARD = 2
};

enum Color
{
    WHITE = 0,
    BLACK = 1,
    EMPTY = 2
};

struct GameSettings
{
    GameStyle style;
    Difficulty difficulty;
    Color color;
    int time, times[2];
    QString name;
    bool has_time;

    QString formatted_time(Color color)
    {
        int ret_time = color == EMPTY ? time : times[color];
        if(ret_time < 0)
        {
            return "00:00";
        }
        QString ret = "";
        if (ret_time / 60 < 10)
        {
            ret += "0";
        }
        ret += QString::number(ret_time/60) + ":";


        if (ret_time % 60 < 10)
        {
            ret += "0";
        }
        ret += QString::number(ret_time % 60);
        return ret;
    }
};

#endif // GAMEUTILS_H
