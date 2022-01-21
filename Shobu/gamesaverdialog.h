#ifndef GAMESAVERDIALOG_H
#define GAMESAVERDIALOG_H

#include "gameloaderdialog.h"

class QLineEdit;

class GameSaverDialog : public GameLoaderDialog
{
    Q_OBJECT
public:
    explicit GameSaverDialog(QWidget *parent = nullptr);

private:
    QLineEdit *_name;

    void onRowChange() override;
};

#endif // GAMESAVERDIALOG_H
