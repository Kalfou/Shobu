#include "gamecontrollerview.h"

#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

#include "gamesaverdialog.h"
#include "gamesettingsdialog.h"
#include "shobumodel.h"

enum SizeDivider // resizing values
{
    BUTTON_WIDTH   = 1,   // space one button gets in tenths
    LABEL_WIDTH    = 3,   // space one label  gets in tenths
    BUTTON_PADDING = 1,   // padding of button width inside its space in tenths (or percents of whole widget)
    FONT_HEIGHT    = 3,   // divide height with this to get font size
    FONT_WIDTH     = 5    // divide width with this to get font size
};

// PUBLIC

// Constructor
GameControllerView::GameControllerView(ShobuModel *model, QWidget *parent) : QWidget(parent), _model(model)
{
    // create time labels
    _time_label[WHITE] = new QLabel("00:00", this);
    _time_label[WHITE]->setAlignment(Qt::AlignCenter);
    _time_label[WHITE]->setObjectName("white");

    _time_label[BLACK] = new QLabel("00:00", this);
    _time_label[BLACK]->setAlignment(Qt::AlignCenter);
    _time_label[BLACK]->setObjectName("black");

    // hide time labels
    _time_label[WHITE]->setVisible(false);
    _time_label[BLACK]->setVisible(false);

    // create buttons
    _stop_time    = new QPushButton(this);
    _save_game    = new QPushButton(this);
    _set_settings = new QPushButton(this);
    _back_to_menu = new QPushButton(this);

    // hide buttons
    _stop_time->setVisible(false);
    _save_game->setVisible(false);
    _set_settings->setVisible(false);
    _back_to_menu->setVisible(false);

    // set button icons
    _stop_time->setIcon(QIcon(QPixmap(":/images/images/pause.png")));
    _save_game->setIcon(QIcon(QPixmap(":/images/images/save.png")));
    _set_settings->setIcon(QIcon(QPixmap(":/images/images/gear.png")));
    _back_to_menu->setIcon(QIcon(QPixmap(":/images/images/leave.png")));

    // connections
    QObject::connect(_stop_time,    &QPushButton::clicked, this, &GameControllerView::stopTimeClicked);
    QObject::connect(_save_game,    &QPushButton::clicked, this, &GameControllerView::saveGameClicked);
    QObject::connect(_set_settings, &QPushButton::clicked, this, &GameControllerView::setSettingsClicked);
    QObject::connect(_back_to_menu, &QPushButton::clicked, this, &GameControllerView::leaveGameClicked);
}

// update widget to current game mode
void GameControllerView::updateLayout()
{
    emptyLayout();
    fillLayout();
    resizeContent();
    refreshTime();
}

// update the current time
void GameControllerView::refreshTime()
{
    if (_model->getSettings().style == SOLO) // only the user has time limit
    {
        Color color = _model->getSettings().color;
        _time_label[color]->setText(_model->getSettings().formatted_time(color));
    }
    else // both have time limit
    {
        _time_label[WHITE]->setText(_model->getSettings().formatted_time(WHITE));
        _time_label[BLACK]->setText(_model->getSettings().formatted_time(BLACK));
    }
}

// make sure the button shows the apropriate icon
void GameControllerView::refreshTimeButton()
{
    if (_model->getTicking())
    {
        _stop_time->setIcon(QIcon(QPixmap(":/images/images/pause.png")));
    }
    else
    {
        _stop_time->setIcon(QIcon(QPixmap(":/images/images/play.png")));
    }
}

// PRIVATE

// Layout controllers
// hide everything from the widget
void GameControllerView::emptyLayout()
{
    while (!_layout.isEmpty())
    {
        _layout[0]->setVisible(false);
        _layout.pop_front();
    }
}

// show buittons and labels that are relevant to the game mode
void GameControllerView::fillLayout()
{
    // add widgets depending on game style, make them visible
    switch (_model->getSettings().style)
    {
    case HOTSEAT: // has every label and button
        if (_model->getSettings().has_time)
        {
            _layout.push_back(_time_label[WHITE]);
            _layout.push_back(_stop_time);
            _layout.push_back(_time_label[BLACK]);

            _time_label[WHITE]->setVisible(true);
            _stop_time->setVisible(true);
            _time_label[BLACK]->setVisible(true);
        }
        _layout.push_back(_save_game);
        _layout.push_back(_set_settings);

        _save_game->setVisible(true);
        _set_settings->setVisible(true);
        break;

    case SOLO: // has only one time label, and all the buttons
        if (_model->getSettings().has_time)
        {
            _layout.push_back(_time_label[_model->getSettings().color]);
            _layout.push_back(_stop_time);

            _time_label[_model->getSettings().color]->setVisible(true);
            _stop_time->setVisible(true);
        }
        _layout.push_back(_save_game);
        _layout.push_back(_set_settings);

        _save_game->setVisible(true);
        _set_settings->setVisible(true);
        break;

    case NETWORK: // has only time limit buttons and labels
        if (_model->getSettings().has_time)
        {
            _layout.push_back(_time_label[WHITE]);
            _layout.push_back(_time_label[BLACK]);

            _time_label[WHITE]->setVisible(true);
            _time_label[BLACK]->setVisible(true);
        }
        break;
    }

    // back to menu is always available regardless of style
    _layout.push_back(_back_to_menu);
    _back_to_menu->setVisible(true);
}

// Button click functions
// stop or start timer
void GameControllerView::stopTimeClicked()
{
    _model->changeTime();
    refreshTimeButton();
}

// leave mid game
void GameControllerView::leaveGameClicked()
{
    // ask for confirmation
    QMessageBox msg(this);
    msg.setText("Are you sure you want to leave?");
    msg.setWindowFlags(Qt::SplashScreen);
    msg.addButton("No", QMessageBox::RejectRole);
    msg.addButton("Yes", QMessageBox::AcceptRole);

    if (msg.exec() == QMessageBox::Accepted)
    {
        if (_model->getTicking()) // if timer is on, stop it
        {
            _model->changeTime();
        }
        if (_model->getSettings().style == NETWORK) // give up if network game
        {
            _model->disconnectFromServer();
        }
        emit leaveGame();
    }
}

// save the current state with a given name
void GameControllerView::saveGameClicked()
{
    GameSaverDialog dialog(this);
    if (dialog.exec() == GameChooserDialog::Accepted && !dialog.getResult().isEmpty())
    {
        // tell user if save was successful
        QMessageBox msg(this);
        msg.setWindowFlags(Qt::SplashScreen);
        if (_model->saveGame(dialog.getResult()))
        {
            msg.setText("Save successful");
        }
        else
        {
            msg.setText("Save failed");
        }
        msg.exec();
    }
}

// change settings mid game with a dialog
void GameControllerView::setSettingsClicked()
{
    GameSettingsDialog dialog(_model->getSettings(), this);

    if (dialog.exec() == GameSettingsDialog::Accepted)
    {
        _model->changeSettings(dialog.getSetting());
        updateLayout();
    }
}

// Resizing
// custom resize event
void GameControllerView::resizeEvent(QResizeEvent*)
{
    resizeContent();
}

// resize buttons and labels based on current widget size
void GameControllerView::resizeContent()
{
    int left = 0; // left parameter of the current widget

    int button_width  = (10-2*BUTTON_PADDING)*width()/100; // real width  of a button
    int button_height = (10-2*BUTTON_PADDING)*height()/10; // real height of a button
    int button_top    = height()/10;

    int icon_width    = 8*button_width/10;   // width of icons
    int icon_height   = 8*button_height/10;  // height of all icons

    QFont font = _time_label[WHITE]->font();
    font.setPointSize(qMin(this->height()/FONT_HEIGHT, _time_label[WHITE]->width()/FONT_WIDTH));

    // Resize widgets

    _time_label[WHITE]->setGeometry(left*width()/100,
                                    0,
                                    LABEL_WIDTH*width()/10,
                                    height());
    _time_label[WHITE]->setFont(font);
    left = left + LABEL_WIDTH;

    _stop_time->setGeometry((left*10+BUTTON_PADDING)*width()/100,
                            button_top,
                            button_width,
                            button_height);
    _stop_time->setIconSize(QSize(icon_width,icon_height));
    left = left + BUTTON_WIDTH;

    _time_label[BLACK]->setGeometry(left*width()/10,
                                    0,
                                    LABEL_WIDTH*width()/10,
                                    height());
    _time_label[BLACK]->setFont(font);
    left = left + LABEL_WIDTH;

    _save_game->setGeometry((left*10+BUTTON_PADDING)*width()/100,
                            button_top,
                            button_width,
                            button_height);
    _save_game->setIconSize(QSize(icon_width,icon_height));
    left = left + BUTTON_WIDTH;

    _set_settings->setGeometry((left*10+BUTTON_PADDING)*width()/100,
                               button_top,
                               button_width,
                               button_height);
    _set_settings->setIconSize(QSize(icon_width,icon_height));
    left = left + BUTTON_WIDTH;

    _back_to_menu->setGeometry((left*10+BUTTON_PADDING)*width()/100,
                               button_top,
                               button_width,
                               button_height);
    _back_to_menu->setIconSize(QSize(icon_width,icon_height));
}
