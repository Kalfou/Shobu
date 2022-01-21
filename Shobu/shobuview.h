#ifndef SHOBUVIEW_H
#define SHOBUVIEW_H

#include <QWidget>

#include "gameutils.h"

class QPushButton;
class QLabel;

class ShobuModel;
class BoardsTable;
class GameControllerView;
class GameSettingsDialog;
class StateControllerView;

class ShobuView : public QWidget
{
    Q_OBJECT

public:
    ShobuView(QWidget *parent = nullptr);
    ~ShobuView();

private:
    ShobuModel *_model;

    QVector<QPushButton*> _main_buttons;
    QVector<QPushButton*> _multi_buttons;

    QVector<QWidget*> _layout;

    // game widgets
    GameControllerView *_game_header;
    BoardsTable *_boards;
    StateControllerView *_game_footer;

    // main menu widgets
    QLabel *_main_title;
    QPushButton *_new_solo;
    QPushButton *_load_game;
    QPushButton *_go_to_multi;
    QPushButton *_exit_game;

    // multi menu widgets
    QLabel *_multi_title;
    QPushButton *_hot_seat;
    QPushButton *_join_online;
    QPushButton *_create_online;
    QPushButton *_back_to_main;

    // wait room widgets
    QPushButton *_leave_wait_room;
    QLabel *_please_wait;

    void resizeEvent(QResizeEvent*) override;
    void resizeMenu(QLabel *title, QVector<QPushButton*> buttons);
    void resizeGame();
    void resizeWaitRoom();

    // layout control
    void emptyLayout();
    void setGameLayout();
    void setMainMenuLayout();
    void setMultiMenuLayout();
    void setWaitRoomLayout();

    // game functions
    void gameOver(Color victor);
    void tick();
    void onBoardChange();
    void onStepGame();

    // main menu functions
    void newSoloClicked();
    void loadGameClicked();
    void goToMultiClicked();

    // multi menu functions
    void hotSeatClicked();
    void joinOnlineClicked();
    void createOnlineClicked();
    void backToMainClicked();

    // wait room
    void leaveWaitRoomClicked();

    // network
    void onServerMessage(QString message);
};
#endif // SHOBUVIEW_H
