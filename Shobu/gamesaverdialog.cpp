#include "gamesaverdialog.h"

#include <QLineEdit>
#include <QVBoxLayout>
#include <QListWidget>

// PUBLIC

// Constructor
GameSaverDialog::GameSaverDialog(QWidget *parent) : GameLoaderDialog(parent)
{
    // add text input box
    _name = new QLineEdit(game_name);
    QFont font = _name->font();
    font.setPointSize(FONT_SIZE);
    _name->setFont(font);

    layout->insertWidget(1, _name);

    QObject::connect(_name, &QLineEdit::textChanged, this, [=](){game_name = _name->text();});
}

// PRIVATE

// update return value on change
void GameSaverDialog::onRowChange()
{
    if (list->currentRow() >= 0)
    {
        game_name = list->currentItem()->text();
        _name->setText(game_name);
    }
    else
    {
        game_name = "";
    }
}
