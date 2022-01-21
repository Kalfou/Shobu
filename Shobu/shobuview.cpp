#include "shobuview.h"

#include <QApplication>
#include <QInputDialog>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

#include "shobumodel.h"
#include "boardstable.h"
#include "gamecontrollerview.h"
#include "statecontrollerview.h"
#include "gameloaderdialog.h"
#include "onlinegamechooserdialog.h"
#include "gamesettingsdialog.h"

enum SizeDivider // contains values to divide available space
{
    TITLE_HEIGHT         = 4,   // divide the widget height() with this to get the title height
    TITLE_FONT_WIDTH     = 10,  // divide the title width() with this to get the font size
    TITLE_FONT_HEIGHT    = 3,   // divide the title height() with this to get the font size
    BUTTON_WIDTH         = 8,   // width of a button in tenths
    BUTTON_FONT_WIDTH    = 15,  // divide the width() of the button with this to get the font size
    BUTTON_FONT_HEIGHT   = 3,   // divide the height() of the button with this to get the font size
    HEADER_FOOTER_HEIGHT = 8,   // divide height() with this to get the height of the header and the footer
    WAIT_TOP             = 40,  // divide height() with this to get where the button starts
    WAIT_LEFT            = 91,  // the percentage of width where the button starts
    WAIT_FONT_WIDTH      = 30,  // divide the widget width() with this to get the font size
    WAIT_FONT_HEIGHT     = 20,  // divide the widget height() with this to get the font size
    MINIMUM_SIZE         = 400  // minimum width and height of the window
};

// PUBLIC

// Constructor
ShobuView::ShobuView(QWidget *parent): QWidget(parent)
{
    setMinimumSize(MINIMUM_SIZE, MINIMUM_SIZE);

    _model = new ShobuModel(this);

    // game widgets
    _game_header = new GameControllerView(_model, this);
    _boards      = new BoardsTable(_model, this);
    _game_footer = new StateControllerView(_model, this);

    _game_header->setVisible(false);
    _boards->setVisible(false);
    _game_footer->setVisible(false);

    // main menu widgets
    _main_title = new QLabel("Shōbu", this);
    QSizePolicy policy = _main_title->sizePolicy();
    policy.setVerticalPolicy(QSizePolicy::Maximum);
    _main_title->setSizePolicy(policy);
    _main_title->setAlignment(Qt::AlignCenter);

    _new_solo    = new QPushButton("New Game", this);
    _load_game   = new QPushButton("Load Game", this);
    _go_to_multi = new QPushButton("Multiplayer", this);
    _exit_game   = new QPushButton("Exit", this);

    _main_buttons.push_back(_new_solo);
    _main_buttons.push_back(_load_game);
    _main_buttons.push_back(_go_to_multi);
    _main_buttons.push_back(_exit_game);

    // multi menu widgets
    _multi_title = new QLabel("Multiplayer", this);
    _multi_title->setSizePolicy(policy); // same as main title
    _multi_title->setAlignment(Qt::AlignCenter);
    _multi_title->setVisible(false);

    _hot_seat      = new QPushButton("Hotseat", this);
    _join_online   = new QPushButton("Join Online Game", this);
    _create_online = new QPushButton("Create Online Game", this);
    _back_to_main  = new QPushButton("Back To Main Menu", this);

    _multi_buttons.push_back(_hot_seat);
    _multi_buttons.push_back(_join_online);
    _multi_buttons.push_back(_create_online);
    _multi_buttons.push_back(_back_to_main);

    for (QPushButton* &button : _multi_buttons) // hide all buttons
    {
        button->setVisible(false);
    }

    // wait room widgets
    _please_wait = new QLabel("Please wait for the other player to join!", this);
    _please_wait->setVisible(false);
    _please_wait->setSizePolicy(policy); // same as menu titles
    _please_wait->setAlignment(Qt::AlignCenter);

    _leave_wait_room = new QPushButton(this);
    _leave_wait_room->setVisible(false);
    _leave_wait_room->setIcon(QIcon(QPixmap(":/images/images/leave.png")));


    setMainMenuLayout(); // add main menu widgets to layout


    // Event handling
    // model
    QObject::connect(_model,  &ShobuModel::stepGame,      this, &ShobuView::onStepGame);
    QObject::connect(_model,  &ShobuModel::boardChange,   this, &ShobuView::onBoardChange);
    QObject::connect(_model,  &ShobuModel::gameOver,      this, &ShobuView::gameOver);
    QObject::connect(_model,  &ShobuModel::timeIsPassing, this, &ShobuView::tick);

    // game layout
    QObject::connect(_game_header, &GameControllerView::leaveGame, this, &ShobuView::setMainMenuLayout);

    // main menu
    QObject::connect(_new_solo,    &QPushButton::clicked, this, &ShobuView::newSoloClicked);
    QObject::connect(_load_game,   &QPushButton::clicked, this, &ShobuView::loadGameClicked);
    QObject::connect(_go_to_multi, &QPushButton::clicked, this, &ShobuView::goToMultiClicked);
    QObject::connect(_exit_game,   &QPushButton::clicked, QApplication::instance(), &QApplication::quit);

    // multi menu
    QObject::connect(_hot_seat,      &QPushButton::clicked, this, &ShobuView::hotSeatClicked);
    QObject::connect(_join_online,   &QPushButton::clicked, this, &ShobuView::joinOnlineClicked);
    QObject::connect(_create_online, &QPushButton::clicked, this, &ShobuView::createOnlineClicked);
    QObject::connect(_back_to_main,  &QPushButton::clicked, this, &ShobuView::backToMainClicked);

    //wait room
    QObject::connect(_leave_wait_room, &QPushButton::clicked, this, &ShobuView::leaveWaitRoomClicked);

    // network
    QObject::connect(_model, &ShobuModel::gotServerMessage,  this, &ShobuView::onServerMessage);
    QObject::connect(_model, &ShobuModel::onlineGameStarted, this, &ShobuView::setGameLayout);


    // Stylesheet
    setStyleSheet("QPushButton {background:#A0522D;color:#FFE4C4}"
                  "QPushButton:disabled {background:#BFA192;color:#FFE4C4}"
                  "QPushButton#draw_icon {background: rgba(0,0,0,0)}"
                  "QLabel:disabled {color:#BFA192}"
                  "QLabel { color:#A0522D;}"
                  "QLabel#white {color: rgb(184, 122, 61)}"
                  "QLabel#black {color: rgb(52, 36, 21)}"
                  "ShobuView {background:#F4F0E8; font: 18px}"
                  "QDialog   {border: 5px solid rgb(52, 36, 21)}"
                  "QLineEdit {border: 3px solid #A0522D; background: #F4F0E8; color: #A0522D; selection-background-color:#A0522D; font: 16px}");
}

// Destructor
ShobuView::~ShobuView(){}

// PRIVATE

// override resize event to better fit our needs
void ShobuView::resizeEvent(QResizeEvent*)
{
    resizeMenu(_main_title, _main_buttons);
    resizeMenu(_multi_title, _multi_buttons);
    resizeGame();
    resizeWaitRoom();
}

// set widget sizes for menus for current window size
void ShobuView::resizeMenu(QLabel *title, QVector<QPushButton*> buttons)
{
    QFont font = title->font();
    title->setGeometry(0,0, width(), height()/TITLE_HEIGHT);
    font.setPointSize(qMin(title->width()/TITLE_FONT_WIDTH, title->height()/TITLE_FONT_HEIGHT));
    title->setFont(font);

    if (buttons.length() == 0) // if we have no butons, we are done
    {
        return;
    }

    int top           = height()/TITLE_HEIGHT;                                       // contains the y value of the button
    int space         = ((TITLE_HEIGHT-1)*height())/(TITLE_HEIGHT*buttons.length()); // the height one button gets
    int button_height = 2*space/3;                                                   // button takes up two thirds of the available space
    int button_width  = BUTTON_WIDTH*this->width()/10;                               // the width of a button

    // set font
    font = buttons[0]->font();
    font.setPointSize(qMin(button_width/BUTTON_FONT_WIDTH, button_height/BUTTON_FONT_HEIGHT));

    // set button sizes and add font
    for (QPushButton* &button : buttons)
    {
        button->setGeometry(this->width()/10,
                                top,
                                button_width,
                                button_height);
        top = top + space;
        button->setFont(font);
    }
}

// set widget sizes for game for current window size
void ShobuView::resizeGame()
{
    _game_header->setGeometry(0,0, width(), height()/HEADER_FOOTER_HEIGHT);
    _boards->setGeometry(0,
                         height()/HEADER_FOOTER_HEIGHT,
                         width(),
                         height()/HEADER_FOOTER_HEIGHT*(HEADER_FOOTER_HEIGHT-2));
    _game_footer->setGeometry(0,
                              height()/HEADER_FOOTER_HEIGHT*(HEADER_FOOTER_HEIGHT-1),
                              width(),
                              height()/HEADER_FOOTER_HEIGHT);
}

// set widget sizes for wait room for current window size
void ShobuView::resizeWaitRoom()
{
    _leave_wait_room->setGeometry(WAIT_LEFT*width()/100,
                                  height()/WAIT_TOP,
                                  BUTTON_WIDTH*width()/100,
                                  height()/10);

    _leave_wait_room->setIconSize(QSize(8*_leave_wait_room->width()/10,8*_leave_wait_room->height()/10));

    _please_wait->setGeometry(0,0,width(), height());

    QFont font = _please_wait->font();
    font.setPointSize(qMin(width()/WAIT_FONT_WIDTH, height()/WAIT_FONT_HEIGHT));
    _please_wait->setFont(font);
}

// Universal layout functions
// remove everything from window
void ShobuView::emptyLayout()
{
    while (!_layout.isEmpty())
    {
        _layout[0]->setVisible(false);
        _layout.pop_front();
    }
}

// remove previous layout, set game layout
void ShobuView::setGameLayout()
{
    emptyLayout();

    // put components into the layout
    _layout.push_back(_game_header);
    _layout.push_back(_boards);
    _layout.push_back(_game_footer);

    // update components
    _game_header->updateLayout();
    _game_footer->updateLayout();

    // make components visible
    _game_header->setVisible(true);
    _boards->setVisible(true);
    _game_footer->setVisible(true);
}

// remove previous layout, set main menu layout
void ShobuView::setMainMenuLayout()
{
    emptyLayout();

    // set up title
    _layout.push_back(_main_title);
    _main_title->setVisible(true);

    // set up buttons
    for (QPushButton* &button : _main_buttons)
    {
        _layout.push_back(button);
        button->setVisible(true);
    }

    resizeMenu(_main_title, _main_buttons);
}

// remove previous layout, set multi menu layout
void ShobuView::setMultiMenuLayout()
{
    emptyLayout();

    // set up title
    _layout.push_back(_multi_title);
    _multi_title->setVisible(true);

    // set up buttons
    for (QPushButton* &button : _multi_buttons)
    {
        _layout.push_back(button);
        button->setVisible(true);
    }

    resizeMenu(_multi_title, _multi_buttons);
}

// remove previous layout, set waitroom layout
void ShobuView::setWaitRoomLayout()
{
    emptyLayout();

    _layout.push_back(_please_wait);
    _please_wait->setVisible(true);

    _layout.push_back(_leave_wait_room);
    _leave_wait_room->setVisible(true);
}

// Game functions
// end game with a winner
void ShobuView::gameOver(Color victor)
{
    _model->endGame();
    QMessageBox msg(this);
    switch (victor)
    {
    case WHITE:
        msg.setText("White won the game!");
        break;
    case BLACK:
        msg.setText("Black won the game");
        break;
    default:
        msg.setText("The game ended with a draw");
        break;
    }
    msg.setWindowFlags(Qt::SplashScreen);
    msg.addButton("Back to main menu", QMessageBox::RejectRole);
    msg.addButton("Restart game", QMessageBox::AcceptRole);

    if (msg.exec() == QMessageBox::Accepted)
    {
        _model->newGame(_model->getSettings());
    }
    else
    {
        setMainMenuLayout();
    }
}

// update time of view
void ShobuView::tick()
{
    _game_header->refreshTime();
}

// update view
void ShobuView::onBoardChange()
{
    _boards->updateBoards();
    _game_footer->refreshButtons();
    _game_header->refreshTimeButton();
}

// update view and request next player to make a move if game is not over
void ShobuView::onStepGame()
{
    onBoardChange();

    GameState *game = _model->getMoveState()->game;

    if (game->getVictor() == game->getOpponent() || !game->hasMoves())
    {
        gameOver(game->getOpponent());
    }
    else
    {
        _model->getPlayer()->makeMove();
    }
}

// Main menu button click functions
// starts new game, asks for settings
void ShobuView::newSoloClicked()
{
    GameSettingsDialog dialog(SOLO, this);
    if (dialog.exec() == GameSettingsDialog::Accepted)
    {
        _model->newGame(dialog.getSetting());
        setGameLayout();
    }
}

// loads a game
void ShobuView::loadGameClicked()
{
    GameLoaderDialog dialog(this);
    if (dialog.exec() == GameChooserDialog::Accepted && !dialog.getResult().isEmpty())
    {
        if (_model->loadGame(dialog.getResult()))
        {
            setGameLayout(); // start game if load successful
        }
        else // notify user if load not successful
        {
            QMessageBox msg(this);
            msg.setText("The save file is corrupted");
            msg.setWindowFlags(Qt::SplashScreen);
            msg.exec();
        }
    }
}

// goes to multi menu
void ShobuView::goToMultiClicked()
{
    setMultiMenuLayout();
}

// Multi menu button click functions
// creates hotseat game, asks for settings
void ShobuView::hotSeatClicked()
{
    GameSettingsDialog dialog(HOTSEAT, this);
    if (dialog.exec() == GameSettingsDialog::Accepted)
    {
        _model->newGame(dialog.getSetting());
        setGameLayout();
    }
}

// connects to server to join to an existing game
void ShobuView::joinOnlineClicked()
{
    const QString hostname = QInputDialog::getText(
        this
        , "Choose Server"
        , "Server Address"
        , QLineEdit::Normal
        , QStringLiteral("127.0.0.1"),
        nullptr,
        Qt::SplashScreen
    );

    if (hostname.isEmpty())
    {
        return;
    }

    if (_model->connectToServer(hostname))
    {
        OnlineGameChooserDialog dialog(_model, this);
        if (dialog.exec() == GameSettingsDialog::Accepted && dialog.getResult() != "")
        {
            _model->joinOnlineGame(dialog.getResult());

            setWaitRoomLayout(); // this might not be visible if the server answers fast enough
        }
        else
        {
            _model->disconnectFromServer(); // disconnect if joining is cancelled
        }
    }
    else // notify user if could not connect to server
    {
        QMessageBox msg(this);
        msg.setText("Could not connect to server!");
        msg.setWindowFlags(Qt::SplashScreen);
        msg.exec();
    }

}

// connects to server, asks for settings, creates online game
void ShobuView::createOnlineClicked()
{
    const QString hostname = QInputDialog::getText(
        this
        , "Choose Server"
        , "Server Address"
        , QLineEdit::Normal
        , QStringLiteral("127.0.0.1"),
        nullptr,
        Qt::SplashScreen
    );

    if (hostname.isEmpty())
    {
        return;
    }

    if (_model->connectToServer(hostname))
    {
        GameSettingsDialog dialog(NETWORK, this);
        if (dialog.exec() == GameSettingsDialog::Accepted && dialog.getSetting().name != "")
        {
            setWaitRoomLayout(); // go to wait room till opponent arrives

            _model->createOnlineGame(dialog.getSetting());
        }
        else
        {
            _model->disconnectFromServer(); // disconnect if creating is cancelled
        }
    }
    else // notify user if could not connect to server
    {
        QMessageBox msg(this);
        msg.setText("Could not connect to server!");
        msg.setWindowFlags(Qt::SplashScreen);
        msg.exec();
    }
}

// goes back to main menu
void ShobuView::backToMainClicked()
{
    setMainMenuLayout();
}

// Wait room button click function
void ShobuView::leaveWaitRoomClicked()
{
    _model->disconnectFromServer();
    setMainMenuLayout();
}


// Network
// notify user of server error and break connection
void ShobuView::onServerMessage(QString message)
{
    // stop game in progress
    if (_model->getTicking())
    {
        _model->changeTime();
    }

    _model->disconnectFromServer();

    //send message
    QMessageBox msg(this);
    msg.setText(message);
    msg.setWindowFlags(Qt::SplashScreen);
    msg.exec();

    //return to menu
    setMultiMenuLayout();
}
