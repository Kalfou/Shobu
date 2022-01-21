#ifndef GAMESETTINGSDIALOG_H
#define GAMESETTINGSDIALOG_H

#include <QDialog>

#include "gameutils.h"

class QGridLayout;
class QPushButton;
class QLabel;
class QCheckBox;
class QLineEdit;

class GameSettingsDialog: public QDialog
{
    Q_OBJECT
public:
    GameSettingsDialog(GameStyle style, QWidget *parent = nullptr);
    GameSettingsDialog(GameSettings settings, QWidget *parent = nullptr);

    // getter
    GameSettings getSetting() const {return _settings;}

private:
    GameSettings _settings;

    // widgets
    QGridLayout *_layout;

    QLineEdit *_name;

    QPushButton *_color;
    QPushButton *_decrease_time;
    QPushButton *_increase_time;
    QPushButton *_decrease_difficulty;
    QPushButton *_increase_difficulty;
    QPushButton *_reject;
    QPushButton *_accept;

    QCheckBox *_has_time;

    QLabel *_box_title;
    QLabel *_color_label;
    QLabel *_has_time_label;
    QLabel *_time;
    QLabel *_difficulty;

    // functions
    void setupWindow();

    void nameChange();
    void colorChange();
    void timeChange();
    void timeIncrease();
    void timeDecrease();
    void difficultyIncrease();
    void difficultyDecrease();
};

#endif // GAMESETTINGSDIALOG_H
