#ifndef BOARDSTABLE_H
#define BOARDSTABLE_H

#include <QWidget>

class QGridLayout;

class ShobuModel;
class Board;

class BoardsTable : public QWidget
{
    Q_OBJECT
public:
    BoardsTable(ShobuModel *model, QWidget *parent = nullptr);

    void updateBoards();

private:
    QGridLayout *_table_layout;
    Board *_boards[4];

    void resizeEvent(QResizeEvent*) override;
};

#endif // BOARDSTABLE_H
