#include "boardstable.h"

#include <QGridLayout>

#include "board.h"

enum ResizeValues // values used for the resizing
{
    SIZE_DIVIDER    =  3, // one board takes up one SIZE_DIVIDERth of the available
    PADDING_DIVIDER = 18  // distance of one board from the center of the boardstable
};

// PUBLIC

// Constructor
BoardsTable::BoardsTable(ShobuModel *model, QWidget *parent) : QWidget(parent)
{
    _table_layout = new QGridLayout(); // create layout

    // create boards
    for (int i = 0; i < 4; ++i)
    {
        _boards[i] = new Board(model, i, this);
    }

    // add boards to layout
    _table_layout->addWidget(_boards[0], 0, 0);
    _table_layout->addWidget(_boards[1], 0, 1);
    _table_layout->addWidget(_boards[2], 1, 0);
    _table_layout->addWidget(_boards[3], 1, 1);

    setLayout(_table_layout); // set layout
}

// boards will show the current state
void BoardsTable::updateBoards()
{
    for (int i = 0; i < 4; ++i)
    {
        _boards[i]->update();
    }
}

// PRIVATE

// resize all boards
void BoardsTable::resizeEvent(QResizeEvent*)
{
    int width_center   = width()/2;  // to get the center, it will always be divided by 2
    int height_center  = height()/2;

    int board_size     = qMin(height(), width())/SIZE_DIVIDER;
    int height_padding = height()/PADDING_DIVIDER;
    int width_padding  = width()/PADDING_DIVIDER;

    // upper left
    _boards[0]->setGeometry(width_center  - width_padding  - board_size,
                            height_center - height_padding - board_size,
                            board_size,
                            board_size);
    // upper right
    _boards[1]->setGeometry(width_center  + width_padding,
                            height_center - height_padding - board_size,
                            board_size,
                            board_size);
    // bottom left
    _boards[2]->setGeometry(width_center  - width_padding - board_size,
                            height_center + height_padding,
                            board_size,
                            board_size);
    // bottom right
    _boards[3]->setGeometry(width_center  + width_padding,
                            height_center + height_padding,
                            board_size,
                            board_size);
}
