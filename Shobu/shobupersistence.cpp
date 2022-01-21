#include "shobupersistence.h"

#include <QFile>
#include <QDir>
#include <QTextStream>

#include "gamestate.h"

// PUBLIC

// Constructor
ShobuPersistence::ShobuPersistence(GameState *state, QObject *parent) : QObject(parent)
{
    _game = state;

    top = -1;
    current = -1;
}

// prepare for a new game
void ShobuPersistence::initialize()
{
    //clear states from previous game and save the starting state
    clearVector();
    saveState();
}

// save current state
void ShobuPersistence::saveState()
{
    // remove saved forward steps
    while (current < top)
    {
        _states[top--]->deleteLater();
        _states.pop_back();
    }
    // save current state
    GameState *new_state = new GameState(this);
    new_state->setState(_game);
    _states.push_back(new_state);
    ++top;
    ++current;
}

// returns a list of avaliable saves in the saves folder
QStringList ShobuPersistence::getSaves()
{
    QDir directory("saves");
    return directory.entryList(QStringList() << "*.shob", QDir::Files);
}

// deletes a .shob file by name if it is in the saves folder
bool ShobuPersistence::deleteSave(QString save)
{
    QFile file = QFile("saves/" + save + ".shob");
    return file.remove();
}

// restore the earlier state
bool ShobuPersistence::undoStep(int backstep)
{
    if (!hasBackState(backstep))
    {
        return false;
    }
    current = current - backstep;
    _game->setState(_states[current]);
    return true;
}

// restore a state that has been undone
bool ShobuPersistence::redoStep(int step)
{
    if (!hasForwardState(step))
    {
        return false;
    }
    current = current + step;
    _game->setState(_states[current]);
    return true;
}

// saves a game to the saves folder with the given name, overwrites if the name is taken
bool ShobuPersistence::saveGame(QString filename, GameSettings settings)
{
    // create folder if it does not exist
    QDir dir;
    if (!dir.exists("saves"))
    {
        dir.mkdir("saves");
    }

    // open file
    QFile file("saves/" + filename + ".shob");
    if (!file.open(QFile::WriteOnly))
    {
        return false; // if we can not open the file for reading, save fails
    }
    QTextStream stream(&file);

    // save style
    switch (settings.style)
    {
    case SOLO:
        stream << SOLO << endl;
        stream << settings.color << endl;
        stream << settings.difficulty << endl;
        break;

    case HOTSEAT:
        stream << HOTSEAT << endl;
        break;

    default:
        return false; // if gamestyle is not HOTSEAT or SOLO, save fails
        break;
    }

    // save time
    stream<<settings.has_time<<endl;
    if (settings.has_time) // continue only if there is time
    {
        stream << settings.time << endl;
        if (settings.style == HOTSEAT)
        {
            stream << settings.times[WHITE] << endl;
            stream << settings.times[BLACK] << endl;
        }
        else
        {
            stream << settings.times[settings.color] << endl;
        }
    }

    // save board state
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                stream << _game->getField(i,j,k) << endl;
            }
        }
    }

    // save turn
    stream << _game->getTurn();

    file.close();

    return true;
}

// load game from the given file
bool ShobuPersistence::loadGame(const QString &filename, GameSettings &settings)
{

    // open file
    QFile file("saves/" + filename + ".shob");
    if (!file.open(QFile::ReadOnly))
    {
        return false; // if we can not open the file for reading, load fails
    }
    QTextStream stream(&file);

    // get game style
    int input = stream.readLine().toInt();
    switch (static_cast<GameStyle>(input))
    {
    case SOLO:
        settings.style = SOLO;
        input = stream.readLine().toInt();
        if (Color color = static_cast<Color>(input); color == WHITE || color == BLACK)
        {
            settings.color = color;
        }
        else
        {
            return false; // if side is not white or black, load fails
        }
        input = stream.readLine().toInt();
        if (Difficulty diff = static_cast<Difficulty>(input); diff == EASY || diff == MEDIUM || diff == HARD)
        {
            settings.difficulty = diff;
        }
        else
        {
            return false; // if difficulty is not valid, load fails
        }
        break;

    case HOTSEAT:
        settings.style = HOTSEAT;
        break;

    default:
        return false; // if gamestyle is not saveable, load fails
        break;
    }

    // load time
    input = stream.readLine().toInt();
    if (input == 1 || input == 0)
    {
        settings.has_time = input;
    }
    else
    {
        return false; // if has time is not a bool, load fails
    }

    if (settings.has_time) // if time limits exist, read them
    {
        input = stream.readLine().toInt();
        if (input > 0)
        {
            settings.time = input;
        }
        else
        {
            return false; // if time is not positive, loading fails
        }

        if (settings.style == SOLO)
        {
            input = stream.readLine().toInt();
            if (input > 0 && input <= settings.time)
            {
                settings.times[settings.color] = input;
            }
            else
            {
                return false; // if time is not positive, or is greater then the max, load fails
            }
        }
        else // if not SOLO, it must be HOTSEAT
        {
            input = stream.readLine().toInt();
            if (input > 0 && input <= settings.time)
            {
                settings.times[WHITE] = input;
            }
            else
            {
                return false; // if white time invalid, loading fails
            }

            input = stream.readLine().toInt();
            if (input > 0 && input < settings.time)
            {
                settings.times[BLACK] = input;
            }
            else
            {
                return false; // if black time invalid, loading fails
            }
        }
    }

    // load board state
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                input = stream.readLine().toInt();
                Color color = static_cast<Color>(input);
                if (color == WHITE || color == BLACK || color == EMPTY)
                {
                    _game->setField(i,j,k,color);
                }
                else
                {
                    return false; // if a field is invalid, load fails
                }
            }
        }
    }

    // load turn
    input = stream.readLine().toInt();
    if (Color color = static_cast<Color>(input); color == WHITE || color == BLACK)
    {
        _game->setTurn(color);
    }
    else
    {
        return false; // if turn is not BLACK or WHITE, load fails
    }

    file.close();

    // if game is not over, load succeeded
    if ((_game->getVictor() == EMPTY) && _game->hasMoves())
    {
        initialize(); // if everything is fine, we can start the game
    }
    else
    {
        return false; // if game is already over, load fails
    }

    return true;
}

// PRIVATE

// remove previous states from vector
void ShobuPersistence::clearVector()
{
    // free memory, then remove pointers
    qDeleteAll(_states);
    _states.clear();

    top = -1;
    current = -1;
}
