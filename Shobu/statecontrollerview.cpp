#include "statecontrollerview.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

#include "shobumodel.h"

enum SizeDivider
{
    BUTTON_PADDING  =  1,  // offset of button from the starting percentage
    BUTTON_WIDTH    =  8,  // width of buttons in percentage
    BUTTON_HEIGHT   =  8,  // height of buttons in tenths
    ICON_WIDTH      =  8,  // width of icons in tenths
    ICON_HEIGHT     =  8,  // height of icons in tenths
    DRAW_WIDTH      = 18,  // width of draw button in percentage
    DRAW_ICON_WIDTH = 18,  // width of draw icon in percenage

    // percentage of width where the button starts
    UNDO_START      =  0,
    REDO_START      = 10,
    WHITE_START     = 30,
    DRAW_START      = 40,
    BLACK_START     = 60,
    RESET_START     = 90
};

// Constructor
StateControllerView::StateControllerView(ShobuModel *model, QWidget *parent) : QWidget(parent), _model(model)
{
    // create buttons
    _undo_move  = new QPushButton(this);
    _redo_move  = new QPushButton(this);
    _reset_move = new QPushButton(this);
    _draw_offer = new QPushButton(this);
    _white_offer = new QPushButton(this);
    _black_offer = new QPushButton(this);

    // add icons to buttons
    _undo_move->setIcon(QIcon(QPixmap(":/images/images/left.png")));
    _redo_move->setIcon(QIcon(QPixmap(":/images/images/right.png")));
    _reset_move->setIcon(QIcon(QPixmap(":/images/images/reset.png")));
    _draw_offer->setIcon(QIcon(QPixmap(":/images/images/draw.png")));
    _white_offer->setIcon(QIcon(QPixmap(":/images/images/white_draw.png")));
    _black_offer->setIcon(QIcon(QPixmap(":/images/images/black_draw.png")));

    //hide buttons
    _undo_move->setVisible(false);
    _redo_move->setVisible(false);
    _reset_move->setVisible(false);
    _draw_offer->setVisible(false);
    _white_offer->setVisible(false);
    _black_offer->setVisible(false);

    // make offer buttons less button like
    _white_offer->setObjectName("draw_icon");
    _black_offer->setObjectName("draw_icon");


    // connections
    QObject::connect(_undo_move, &QPushButton::clicked, _model, &ShobuModel::undoStep);
    QObject::connect(_redo_move, &QPushButton::clicked, _model, &ShobuModel::redoStep);
    QObject::connect(_reset_move,&QPushButton::clicked, _model, &ShobuModel::resetMove);
    QObject::connect(_draw_offer,&QPushButton::clicked, _model, &ShobuModel::offerDraw);
}

// enable buttons that are clickable based on model and show draw offers
void StateControllerView::refreshButtons()
{
    _undo_move->setEnabled(_model->hasUndo());
    _redo_move->setEnabled(_model->hasRedo());
    _reset_move->setEnabled(_model->getMoveState()->passive_set);
    _white_offer->setVisible(_model->getDrawOffer(WHITE));
    _draw_offer->setEnabled(!_model->getDrawOffer());
    _black_offer->setVisible(_model->getDrawOffer(BLACK));
}

// show buttons appropriate to the game mode
void StateControllerView::updateLayout()
{
    emptyLayout();
    fillLayout();
    resizeContent();
    refreshButtons();
}

// make all buttons invisible
void StateControllerView::emptyLayout()
{
    while (!_layout.isEmpty())
    {
        _layout[0]->setVisible(false);
        _layout.pop_front();
    }
}

// make apropriate buttons visible, store them in layout vector
void StateControllerView::fillLayout()
{
    if (_model->getSettings().style == SOLO || _model->getSettings().style == HOTSEAT) // undo and redo only in local game
    {
        _layout.push_back(_undo_move);
        _layout.push_back(_redo_move);

        _undo_move->setVisible(true);
        _redo_move->setVisible(true);
    }
    else if (_model->getSettings().style == NETWORK)  // draw only in ponline game
    {
        _layout.push_back(_draw_offer);

        _draw_offer->setVisible(true);
    }
    _layout.push_back(_reset_move);

    _reset_move->setVisible(true);  // reset is always needed
}

// manually keep the widget responsive
void StateControllerView::resizeEvent(QResizeEvent*)
{
    resizeContent();
}

// set size of all buttons based on widget size
void StateControllerView::resizeContent()
{
    int button_width  = BUTTON_WIDTH*width()/100;        // width of normal buttons (not _draw_offer)
    int button_height = BUTTON_HEIGHT*height()/10;       // height of all buttons
    int button_top    = (height()-button_height)/2;      // y coordinate of top of all buttons

    int icon_width      = ICON_WIDTH*button_width/10;    // width of icons (except _draw_offer icon)
    int draw_icon_width = DRAW_WIDTH*button_width/10;    // width of _draw_offer icon
    int icon_height     = ICON_HEIGHT*button_height/10;  // height of all icons

    // set geometry and icon size of each button
    _undo_move->setGeometry((UNDO_START+BUTTON_PADDING)*width()/100,
                            button_top,
                            button_width,
                            button_height);
    _undo_move->setIconSize(QSize(icon_width,icon_height));

    _redo_move->setGeometry((REDO_START+BUTTON_PADDING)*width()/100,
                            button_top,
                            button_width,
                            button_height);
    _redo_move->setIconSize(QSize(icon_width,icon_height));

    _white_offer->setGeometry((WHITE_START+BUTTON_PADDING)*width()/100,
                            button_top,
                            button_width,
                            button_height);
    _white_offer->setIconSize(QSize(icon_width,icon_height));

    _draw_offer->setGeometry((DRAW_START+BUTTON_PADDING)*width()/100,
                            button_top,
                            DRAW_WIDTH*width()/100,
                            button_height);
    _draw_offer->setIconSize(QSize(draw_icon_width,icon_height));

    _black_offer->setGeometry((BLACK_START+BUTTON_PADDING)*width()/100,
                            button_top,
                            button_width,
                            button_height);
    _black_offer->setIconSize(QSize(icon_width,icon_height));

    _reset_move->setGeometry((RESET_START+BUTTON_PADDING)*width()/100,
                            button_top,
                            button_width,
                            button_height);
    _reset_move->setIconSize(QSize(icon_width,icon_height));
}
