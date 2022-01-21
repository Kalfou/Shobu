#include "board.h"

#include <QMouseEvent>
#include <QPainter>

#include "shobumodel.h"

// PUBLIC

// Constructor
Board::Board(ShobuModel *model, int index, QWidget *parent) : QWidget(parent), _model(model), _board_index(index) {}

// PRIVATE

// find the field the player has selected
void Board::mousePressEvent(QMouseEvent* event)
{
    int row    = event->y() * 4 / height();
    int column = event->x() * 4 / width();

    // send click to player
    if (_model->getSettings().style == HOTSEAT || _model->getSettings().color == _model->getMoveState()->game->getTurn())
    {
        _model->getPlayer()->makeMove(_board_index, row, column);
    }
}

// show current position of the board
void Board::paintEvent(QPaintEvent*)
{
    paintBackground();

    // only when we want user input
    if (_model->getSettings().style == HOTSEAT || _model->getSettings().color == _model->getMoveState()->game->getTurn())
    {
        paintSelected();
        paintChoices();
    }
    paintGrid();
    paintPieces();
}

// background color of the board
void Board::paintBackground()
{
    QPainter painter(this);

    // the color of the board depends on the _board_index
    if (_board_index%2)
    {
        painter.setBrush(QColor(247,231,206)); // homeboard of white is light colored
    }
    else
    {
        painter.setBrush(QColor(195, 162, 124)); // homeboard of black is dark
    }

    painter.drawRect(
        0, 0,
        this->width(), this->height());
}

// mark already set parts of the move
void Board::paintSelected()
{
    QPainter painter(this);
    painter.setBrush(QColor(94, 49, 40)); // marked fields are redish brown

    if (_model->getMoveState()->passive_set && _model->getMoveState()->move.p.board == _board_index)
    {
        // mark passive piece
        painter.drawRect(
            _model->getMoveState()->move.p.column * width() / 4,
            _model->getMoveState()->move.p.row * height() / 4,
            this->width()/4,
            this->height()/4);

        if (_model->getMoveState()->vector_set)
        {
            //mark passive destination
            painter.drawRect(
                (_model->getMoveState()->move.p.column + _model->getMoveState()->move.col_change * _model->getMoveState()->move.magnitude) * width() / 4,
                (_model->getMoveState()->move.p.row + _model->getMoveState()->move.row_change * _model->getMoveState()->move.magnitude) * height() / 4,
                this->width()/4,
                this->height()/4);
        }
    }
}

// mark possible choices based on already set move parts
void Board::paintChoices()
{
    QPainter painter(this);
    painter.setBrush(QColor(111, 118, 50)); // available fields are olive green

    if (_model->getMoveState()->passive_set)
    {
        if (_model->getMoveState()->vector_set)
        {
            // get possible agressive pieces
            QVector<Coordinate> agressives =_model->getMoveState()->game->getAgressivePieces(_board_index,
                                                                                  _model->getMoveState()->move.p,
                                                                                  _model->getMoveState()->move.row_change,
                                                                                  _model->getMoveState()->move.col_change,
                                                                                  _model->getMoveState()->move.magnitude
                                                                                  );
            for (Coordinate agressive : agressives) // mark agressive pieces
            {
                // mark agressive pieces
                painter.drawRect(
                    agressive.column * width() / 4,
                    agressive.row    * height() / 4,
                    this->width()/4,
                    this->height()/4);
            }
        }
        else
        {
            // get possible passive destinations
            QVector<Coordinate> destinations =_model->getMoveState()->game->getDestinations(_board_index, _model->getMoveState()->move.p);
            for (Coordinate destination : destinations)
            {
                // mark passive destinations
                painter.drawRect(
                    destination.column * width() / 4,
                    destination.row    * height() / 4,
                    this->width()/4,
                    this->height()/4);
            }
        }
    }
    else
    {
        // get possible passive pieces
        QVector<Coordinate> passives =_model->getMoveState()->game->getPassivePieces(_board_index);
        for (Coordinate passive : passives)
        {
            // mark passive pieces
            painter.drawRect(
                passive.column * width() / 4,
                passive.row    * height() / 4,
                this->width()/4,
                this->height()/4);
        }
    }
}

// paints the grid of the board.
void Board::paintGrid()
{
    for (int i = 1; i < 4; i++) // Shobu is always 4x4
    {
        // columns
        paintLine(QLineF(
            i * width() / 4,
            0,
            i * width() / 4,
            height()),
            Qt::black,
            2);

        // rows
        paintLine(QLineF(
            0,
            i * height() / 4,
            width(),
            i * height() / 4),
            Qt::black,
            2);
    }
}

// used by paintgrid to paint a line
void Board::paintLine(QLineF arg_line, QColor color, int line_width)
{
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(color, line_width));

    painter.drawLine(arg_line);
}

// place the pieces on the board
void Board::paintPieces()
{
    QPainter painter(this);
    for (int i = 0; i < 4; i++) // check every field on the board
    {
        for (int j = 0; j < 4; j++)
        {
            if (_model->getMoveState()->game->getField(_board_index, i, j) == BLACK)
            {
                // black pieces are dark
                painter.setBrush(QBrush(QColor(52, 36, 21)));
                painter.drawEllipse((8*j+1)*width()/32, (8*i+1)*height()/32, 3*width()/16, 3*height()/16);
            }
            else if (_model->getMoveState()->game->getField(_board_index, i, j) == WHITE)
            {
                // white pieces are light
                painter.setBrush(QBrush(QColor(184, 122, 61)));
                painter.drawEllipse((8*j+1)*width()/32, (8*i+1)*height()/32, 3*width()/16, 3*height()/16);
            }
            // empty fields do not get anything
        }
    }
}
