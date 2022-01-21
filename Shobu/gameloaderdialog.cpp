#include "gameloaderdialog.h"

#include <QDir>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QListWidget>

#include "shobupersistence.h"

// PUBLIC

// Constructor
GameLoaderDialog::GameLoaderDialog(QWidget *parent) : GameChooserDialog(parent)
{
    // list saved games
    fillList();

    // add delete button
    _delete_save = new QPushButton("Delete", this);
    QFont font = _delete_save->font();
    font.setPointSize(FONT_SIZE);
    _delete_save->setFont(font);
    buttons->insertWidget(1, _delete_save);

    // connections
    QObject::connect(list, &QListWidget::currentRowChanged, this, &GameLoaderDialog::onRowChange);
    QObject::connect(_delete_save, &QPushButton::clicked, this, &GameLoaderDialog::onDeleteSave);

    // set first row as selected
    if (list->currentRow() >= 0)
    {
        game_name = list->currentItem()->text();
    }
    else
    {
        game_name = "";
    }
}

// PRIVATE

// deletes the save of the current row
void GameLoaderDialog::onDeleteSave()
{
    if (list->currentRow() >= 0) // current row exists
    {
        // ask for confirmation
        QMessageBox msg(this);
        msg.setText("Are you sure you want delete this save?");
        msg.setWindowFlags(Qt::SplashScreen);
        msg.addButton("No", QMessageBox::RejectRole);
        msg.addButton("Yes", QMessageBox::AcceptRole);

        if (msg.exec() == QMessageBox::Accepted)
        {
            ShobuPersistence::deleteSave(list->currentItem()->text());
            fillList();
        }
    }
}

// update return value on change
void GameLoaderDialog::onRowChange()
{
    if (list->currentRow() >= 0)
    {
        game_name = list->currentItem()->text();
    }
    else
    {
        game_name = "";
    }
}

// add new list elements to list, remove old ones
void GameLoaderDialog::fillList()
{
    list->clear();

    // list saved games
    QStringList saves = ShobuPersistence::getSaves();

    for (int i = 0; i < saves.length(); i++)
    {
        list->addItem(saves[i].left(saves[i].lastIndexOf('.')));
    }

    // first one is selected if it exists
    if (!saves.isEmpty())
    {
        list->setCurrentRow(0);
    }
}
