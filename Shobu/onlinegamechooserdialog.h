#ifndef ONLINEGAMECHOOSERDIALOG_H
#define ONLINEGAMECHOOSERDIALOG_H

#include "gamechooserdialog.h"

class ShobuModel;
struct GameSettings;

class OnlineGameChooserDialog : public GameChooserDialog
{
    Q_OBJECT
public:
    explicit OnlineGameChooserDialog(ShobuModel *model, QWidget *parent = nullptr);

private:
    ShobuModel *_model;

    QVector<GameSettings> _settings_list;

    QPushButton *_reload_button;

    void refresh(QVector<GameSettings> new_list);
    void onRowChanged(int current_row);
};

#endif // ONLINEGAMECHOOSERDIALOG_H
