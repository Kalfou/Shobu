#include "gamesettingsdialog.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>

#include "gameutils.h"

enum DialogValues // default values
{
    STARTINGTIME = 300,
    NAMELENGTH = 15
};

// PUBLIC

// Constructors
// with settings
GameSettingsDialog::GameSettingsDialog(GameSettings settings, QWidget *parent) : QDialog(parent)
{
    _settings = settings;
    setupWindow();
}

// Constructor
// if only style is given, we add a default setting
GameSettingsDialog::GameSettingsDialog(GameStyle style, QWidget *parent) : QDialog(parent)
{
    _settings.style      = style;
    _settings.name       = "ShobuGame";
    _settings.color      = WHITE;
    _settings.difficulty = MEDIUM;
    _settings.has_time   = false;
    _settings.time       = STARTINGTIME;
    _settings.times[0]   = STARTINGTIME;
    _settings.times[1]   = STARTINGTIME;

    setupWindow();
}

// PRIVATE

// create widgets for the given game mode
void GameSettingsDialog::setupWindow()
{
    int current_row = 0;

    _layout = new QGridLayout();

    // title
    _box_title = new QLabel("Game Settings");
    _box_title->setAlignment(Qt::AlignCenter);
    QFont font = _box_title->font();
    font.setPointSize(qMin(height(), width()));
    _box_title->setFont(font);
    _layout->addWidget(_box_title,current_row++,0,1,3);

    // name setter
    if (_settings.style == NETWORK)
    {
        _name = new QLineEdit(_settings.name);
        _name->setMaxLength(NAMELENGTH);
        QObject::connect(_name, &QLineEdit::textChanged, this, &GameSettingsDialog::nameChange);
        _layout->addWidget(_name,current_row++,0,1,3,Qt::AlignCenter);
        _name->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);
        _name->setGeometry(width()/10,_name->y(), 8*width()/10, _name->height());
    }

    // color setter
    _color_label = new QLabel("Color:");
    font = _color_label->font();
    font.setPointSize(qMin(width()/2, height()/2));
    _color_label->setFont(font);

    _color = new QPushButton(this);
    _color->setFixedWidth(50);
    _color->setFixedHeight(50);
    if (_settings.color == WHITE)
    {
        _color->setStyleSheet("background:rgb(184, 122, 61);padding:0");
    }
    else
    {
        _color->setStyleSheet("background:rgb(52, 36, 21);padding:0");
    }

    QObject::connect(_color, &QPushButton::clicked, this, &GameSettingsDialog::colorChange);
    _layout->addWidget(_color_label,current_row,0,1,1,Qt::AlignCenter);
    _layout->addWidget(_color,current_row++,1,1,1,Qt::AlignCenter);
    // color only needed when there is one local player
    if (_settings.style == HOTSEAT)
    {
        _color_label->setEnabled(false);
        _color_label->setVisible(false);

        _color->setEnabled(false);
        _color->setVisible(false);
    }

    // time setters
    _has_time_label = new QLabel("Time:");
    _has_time_label->setFont(font);

    _has_time = new QCheckBox();
    _has_time->setChecked(_settings.has_time);
    QObject::connect(_has_time, &QCheckBox::stateChanged, this, &GameSettingsDialog::timeChange);
    _layout->addWidget(_has_time_label,current_row,0,1,1,Qt::AlignCenter);
    _layout->addWidget(_has_time,    current_row++,1,1,1,Qt::AlignCenter);

    _decrease_time = new QPushButton();
    _decrease_time->setIcon(QIcon(QPixmap(":/images/images/left.png")));
    _decrease_time->setEnabled(_settings.has_time);
    _decrease_time->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    QObject::connect(_decrease_time, &QPushButton::clicked, this, &GameSettingsDialog::timeDecrease);
    _layout->addWidget(_decrease_time,current_row,0,1,1,Qt::AlignRight);

    _time = new QLabel(_settings.formatted_time(EMPTY));
    _time->setFont(font);
    _time->setEnabled(_settings.has_time);
    _layout->addWidget(_time,current_row,1,1,1,Qt::AlignCenter);

    _increase_time = new QPushButton();
    _increase_time->setIcon(QIcon(QPixmap(":/images/images/right.png")));
    _increase_time->setEnabled(_settings.has_time);
    QObject::connect(_increase_time, &QPushButton::clicked, this, &GameSettingsDialog::timeIncrease);
    _layout->addWidget(_increase_time,current_row,2,1,1,Qt::AlignLeft);
    ++current_row;

    // difficulty setters
    _decrease_difficulty = new QPushButton();
    _decrease_difficulty->setIcon(QIcon(QPixmap(":/images/images/left.png")));
    QObject::connect(_decrease_difficulty, &QPushButton::clicked, this, &GameSettingsDialog::difficultyDecrease);
    _layout->addWidget(_decrease_difficulty,current_row,0,1,1,Qt::AlignRight);

    QString difficulty_text[3] = {"EASY", "MEDIUM", "HARD"};
    _difficulty = new QLabel(difficulty_text[_settings.difficulty]);
    _difficulty->setFont(font);
    _difficulty->setAlignment(Qt::AlignCenter);
    _layout->addWidget(_difficulty,current_row,1,1,1,Qt::AlignCenter);

    _increase_difficulty = new QPushButton();
    _increase_difficulty->setIcon(QIcon(QPixmap(":/images/images/right.png")));
    QObject::connect(_increase_difficulty, &QPushButton::clicked, this, &GameSettingsDialog::difficultyIncrease);
    _layout->addWidget(_increase_difficulty,current_row,2,1,1,Qt::AlignLeft);
    ++current_row;
    // difficulty only needed in solo mode
    if (_settings.style != SOLO)
    {
        _decrease_difficulty->setEnabled(false);
        _difficulty->setEnabled(false);
        _increase_difficulty->setEnabled(false);

        _decrease_difficulty->setVisible(false);
        _difficulty->setVisible(false);
        _increase_difficulty->setVisible(false);
    }

    // submit buttons
    _reject = new QPushButton("Cancel");
    _reject->setFont(font);
    QObject::connect(_reject, &QPushButton::clicked, this, &GameSettingsDialog::reject);
    _layout->addWidget(_reject,++current_row,0,1,1,Qt::AlignCenter);

    _accept = new QPushButton("Start");
    _accept->setFont(font);
    QObject::connect(_accept, &QPushButton::clicked, this, &GameSettingsDialog::accept);
    _layout->addWidget(_accept,current_row++,2,1,1,Qt::AlignCenter);

    _layout->setSpacing(10);

    setWindowFlags(Qt::SplashScreen);

    setLayout(_layout);
}

// update name of return value
void GameSettingsDialog::nameChange()
{
    if (_name->text().length() <= NAMELENGTH)
    {
        _settings.name = _name->text();
    }
    else
    {
        _name->setText(_settings.name);
    }
}

// update color of return value
void GameSettingsDialog::colorChange()
{
    if (_settings.color == BLACK)
    {
        _settings.color = WHITE;
        _color->setStyleSheet("background:rgb(184, 122, 61);padding:0");
    }
    else
    {
        _settings.color = BLACK;
        _color->setStyleSheet("background:rgb(52, 36, 21);padding:0");
    }
}

// update has_time of return value
void GameSettingsDialog::timeChange()
{
    _settings.has_time = !_settings.has_time;
    _increase_time->setEnabled(_settings.has_time);
    _time->setEnabled(_settings.has_time);
    _decrease_time->setEnabled(_settings.has_time);
}

// increase time of return value
void GameSettingsDialog::timeIncrease()
{
    _decrease_time->setEnabled(true);
    _settings.time = _settings.time + 30;
    _settings.times[0] = _settings.time;
    _settings.times[1] = _settings.time;
    _time->setText(_settings.formatted_time(EMPTY)); // color independent time
}

// decrease time of return value
void GameSettingsDialog::timeDecrease()
{
    _settings.time = _settings.time - 30;
    _settings.times[0] = _settings.time;
    _settings.times[1] = _settings.time;
    if (_settings.time <= 30)
    {
        _decrease_time->setEnabled(false);
    }
    _time->setText(_settings.formatted_time(EMPTY)); // color independent time
}

// increase difficulty of return value
void GameSettingsDialog::difficultyIncrease()
{
    if (_settings.difficulty == EASY)
    {
        _settings.difficulty = MEDIUM;
        _difficulty->setText("MEDIUM");
        _decrease_difficulty->setEnabled(true);
    }
    else
    {
        _settings.difficulty = HARD;
        _difficulty->setText("HARD");
        _increase_difficulty->setEnabled(false);
    }
}

// decrease difficulty of return value
void GameSettingsDialog::difficultyDecrease()
{
    if (_settings.difficulty == HARD)
    {
        _settings.difficulty = MEDIUM;
        _difficulty->setText("MEDIUM");
        _increase_difficulty->setEnabled(true);
    }
    else
    {
        _settings.difficulty = EASY;
        _difficulty->setText("EASY");
        _decrease_difficulty->setEnabled(false);
    }
}
