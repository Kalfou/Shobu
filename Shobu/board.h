#ifndef BOARD_H
#define BOARD_H

#include <QWidget>

class ShobuModel;
class QMouseEvent;

class Board : public QWidget
{
    Q_OBJECT
public:
    Board(ShobuModel *model, int index, QWidget *parent);

private:
    ShobuModel *_model;
    int _board_index;

    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent*) override;

    void paintBackground();
    void paintSelected();
    void paintChoices();
    void paintGrid();
    void paintLine(QLineF arg_line, QColor color, int line_width);
    void paintPieces();
};

#endif // BOARD_H
