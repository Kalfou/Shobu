#ifndef GAMECHOOSERDIALOG_H
#define GAMECHOOSERDIALOG_H

#include <QDialog>

class QHBoxLayout;
class QVBoxLayout;
class QPushButton;
class QListWidget;

class GameChooserDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GameChooserDialog(QWidget *parent = nullptr);

    QString getResult() const {return game_name;}

protected:
    QVBoxLayout *layout;
    QHBoxLayout *buttons;

    QPushButton *ok_button;
    QPushButton *cancel_button;
    QListWidget *list;

    QString game_name;

    enum GameChooserValues
    {
        FONT_SIZE = 12
    };
};

#endif // GAMECHOOSERDIALOG_H
