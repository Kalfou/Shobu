#ifndef STATECONTROLLERVIEW_H
#define STATECONTROLLERVIEW_H

#include <QWidget>

class QHBoxLayout;
class QLabel;
class QPushButton;

class ShobuModel;

class StateControllerView : public QWidget
{
    Q_OBJECT
public:
    StateControllerView(ShobuModel *model, QWidget *parent = nullptr);

    void updateLayout();
    void refreshButtons();

private:
    ShobuModel *_model;

    QVector<QWidget*> _layout; // contains the currently visible widgets except draw offers

    QPushButton *_undo_move;
    QPushButton *_redo_move;
    QPushButton *_reset_move;
    QPushButton *_draw_offer;

    QPushButton *_white_offer;
    QPushButton *_black_offer;

    //layout controllers
    void emptyLayout();
    void fillLayout();

    void resizeEvent(QResizeEvent*) override;
    void resizeContent();
};

#endif // STATECONTROLLERVIEW_H
