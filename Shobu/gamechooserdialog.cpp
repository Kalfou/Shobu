#include "gamechooserdialog.h"

#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>

enum GameChooserValues
{
    FIXED_WIDTH  = 300, // width of the window
    FIXED_HEIGHT = 200  // height of the window
};

// PUBLIC

// Constructor
GameChooserDialog::GameChooserDialog(QWidget *parent) : QDialog(parent)
{
    setFixedSize(FIXED_WIDTH, FIXED_HEIGHT);

    // create list
    list = new QListWidget(this);

    // set list style
    list->setStyleSheet("QListWidget {border: 3px solid #A0522D; background: #F4F0E8; color: #A0522D}"
                        "QListWidget::item:hover {background: rgb(184, 122, 61); color: white}"
                        "QListWidget::item:selected {background: #A0522D; border: 2px solid black}");
    QFont font = list->font();
    font.setPointSize(FONT_SIZE);
    list->setFont(font);

    // create buttons
    ok_button = new QPushButton("Ok",this);
    ok_button->setFont(font);
    cancel_button = new QPushButton("Cancel", this);
    cancel_button->setFont(font);

    buttons = new QHBoxLayout();
    buttons->addWidget(cancel_button);
    buttons->addWidget(ok_button);

    // set up layout
    layout = new QVBoxLayout(this);
    layout->addWidget(list);
    layout->addLayout(buttons);

    setLayout(layout);
    layout->setSpacing(20);

    game_name = ""; // set return valuue to empty

    setWindowFlags(Qt::SplashScreen); // remove header

    // connections
    connect(ok_button,     &QPushButton::clicked, this, &GameChooserDialog::accept);
    connect(cancel_button, &QPushButton::clicked, this, &GameChooserDialog::reject);
}
