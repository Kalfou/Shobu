#ifndef GAMELOADERDIALOG_H
#define GAMELOADERDIALOG_H

#include "gamechooserdialog.h"

class GameLoaderDialog : public GameChooserDialog
{
    Q_OBJECT
public:
    explicit GameLoaderDialog(QWidget *parent = nullptr);

private:
    QPushButton *_delete_save;

    void onDeleteSave();
    virtual void onRowChange();
    void fillList();
};

#endif // GAMELOADERDIALOG_H
