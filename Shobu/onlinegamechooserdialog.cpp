#include "onlinegamechooserdialog.h"

#include <QMessageBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QListWidget>

#include "shobumodel.h"

// PUBLIC

// Constructor
OnlineGameChooserDialog::OnlineGameChooserDialog(ShobuModel *model, QWidget *parent) : GameChooserDialog(parent), _model(model)
{
    // ad reload button
    _reload_button = new QPushButton("Reload", this);
    QFont font = _reload_button->font();
    font.setPointSize(FONT_SIZE);
    _reload_button->setFont(font);

    buttons->insertWidget(1, _reload_button);

    // connections
    QObject::connect(_reload_button, &QPushButton::clicked, _model, &ShobuModel::getOnlineGameList);
    QObject::connect(_model, &ShobuModel::gotOnlineList, this, &OnlineGameChooserDialog::refresh);
    QObject::connect(list, &QListWidget::currentRowChanged, this, &OnlineGameChooserDialog::onRowChanged);

    _model->getOnlineGameList();
}

// PRIVATE

// refreshes the list with the given elements
void OnlineGameChooserDialog::refresh(QVector<GameSettings> new_list)
{
    _settings_list = new_list;
    list->clear();
    game_name = "";

    if (new_list.isEmpty()) // notify user if no games are available
    {
        QMessageBox msg(this);
        msg.setText("No available games found");
        msg.setWindowFlags(Qt::SplashScreen);
        msg.exec();
    }
    else // show user the available games and base settings
    {
        for (int i = 0; i < new_list.length(); ++i)
        {
            QString entry = new_list[i].name;
            entry.append(" | ");
            entry.append(new_list[i].color == WHITE ? "WHITE" : "BLACK"); // the color the user will take in this game
            entry.append(" | ");
            if (new_list[i].has_time)
            {
                entry.append(new_list[i].formatted_time(EMPTY)); // get universal time to show time limit
            }
            else
            {
                entry.append("-"); // show that game has no time limit
            }

            list->addItem(entry);
        }
        list->setCurrentRow(0);
    }
}

// updates the return value on change
void OnlineGameChooserDialog::onRowChanged(int current_row)
{
    if (current_row < 0 || current_row >= _settings_list.length())
    {
        game_name = "";
    }
    else
    {
        game_name = _settings_list[current_row].name;
    }
}
