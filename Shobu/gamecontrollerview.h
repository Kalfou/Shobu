#ifndef GAMECONTROLLERVIEW_H
#define GAMECONTROLLERVIEW_H

#include <QWidget>

class QLabel;
class QPushButton;

class ShobuModel;

class GameControllerView : public QWidget
{
    Q_OBJECT
public:
    GameControllerView(ShobuModel *model, QWidget *parent);

    void updateLayout();
    void refreshTime();
    void refreshTimeButton();

private:
    ShobuModel *_model;

    QVector<QWidget*> _layout;

    QLabel *_time_label[2];

    QPushButton *_stop_time;
    QPushButton *_save_game;
    QPushButton *_set_settings;
    QPushButton *_back_to_menu;

    // layout controllers
    void emptyLayout();
    void fillLayout();

    // button click functions
    void stopTimeClicked();
    void leaveGameClicked();
    void saveGameClicked();
    void setSettingsClicked();

    // resizing
    void resizeEvent(QResizeEvent*) override;
    void resizeContent();

signals:
    void leaveGame();
};

#endif // GAMECONTROLLERVIEW_H
