#include "shobumodel.h"

#include <QTimer>

#include "organicplayer.h"
#include "machineplayer.h"
#include "shobuclient.h"

enum ModelValues
{
    TICK_TIME = 1000,     // miliseconds needed for one tick
    MACHINE_INTERVAL = 2  // ticks needed before machine step
};

// PUBLIC

// Constructor
ShobuModel::ShobuModel(QObject *parent) : QObject(parent)
{
    _timer = new QTimer(this);
    _timer->setInterval(TICK_TIME);
    QObject::connect(_timer, &QTimer::timeout, this, &ShobuModel::tick);

    _game = new GameState(this);
    _move = new MoveState();
    _move->game = _game;

    _persistence = new ShobuPersistence(_game, this);

    // network
    _client = new ShobuClient(this);
    QObject::connect(_client, &ShobuClient::gamesArrived,  this, &ShobuModel::gotOnlineList);
    QObject::connect(_client, &ShobuClient::serverMessage, this, &ShobuModel::gotServerMessage);
    QObject::connect(_client, &ShobuClient::gameStarted,   this, &ShobuModel::onGameStarted);
    QObject::connect(_client, &ShobuClient::drawOffer,     this, &ShobuModel::onDrawOffer);

    // no player type yet
    _players[WHITE] = nullptr;
    _players[BLACK] = nullptr;
}

// Game controllers
// starts a new game
void ShobuModel::newGame(GameSettings settings)
{
    _settings = settings;
    _game->initializeGame(); // initialize boards, white turn

    // time is set to maximum
    if (_settings.has_time)
    {
        _settings.times[WHITE] = _settings.time;
        _settings.times[BLACK] = _settings.time;
    }

    initializeGame();
}

// apply user move or start delay for machine move
void ShobuModel::makeMove()
{
    // if player has turn, step happens immediately
    if (_settings.style == HOTSEAT || _settings.style == NETWORK || _settings.color == _game->getTurn())
    {
        applyMove();
    }
    else // if machine has turn, delay is needed
    {
        _move_ready = true;
        _tick_count = 0;
    }
}

// removes move in progress
void ShobuModel::resetMove()
{
    // if it is our turn, erase move in progress
    if (_settings.style == HOTSEAT || _game->getTurn() == _settings.color)
    {
        if (_move->passive_set)
        {
            _move->vector_set  = false;
            _move->passive_set = false;
            emit boardChange(); // update affected board
        }
    }
}

// stops measuring time
void ShobuModel::endGame()
{
    _timer->stop();
    _ticking = false;
}

// change settings mid game
void ShobuModel::changeSettings(GameSettings settings)
{
    _timer->stop(); // we will restart it later

    int passed_time[2] = {0};

    // no move is in progress
    _move->passive_set = false;
    _move->vector_set  = false;
    _move_ready        = false;

    if (_settings.has_time)
    {
        // get passed time
        passed_time[WHITE] = _settings.time - _settings.times[WHITE];
        passed_time[BLACK] = _settings.time - _settings.times[BLACK];
    }

    _settings = settings;

    if (_settings.has_time)
    {
        // subtract passed time from new time
        _settings.times[WHITE] = _settings.time - passed_time[WHITE];
        _settings.times[BLACK] = _settings.time - passed_time[BLACK];
    }

    applySettings(); // apply modified settings

    // player can make a move
    _timer->start();
    emit stepGame();
}

// Persistence
// saves current game with the given name
bool ShobuModel::saveGame(const QString filename)
{
    return _persistence->saveGame(filename, _settings);
}

bool ShobuModel::loadGame(const QString filename)
{
    if (!_persistence->loadGame(filename, _settings))
    {
        return false;
    }
    initializeGame();
    return true;
}

// restores previous gamestate
bool ShobuModel::undoStep()
{
    int backstep = getStepSize();

    if (!backstep) // can not have 0 as backstep
    {
        return false;
    }
    if (!_persistence->undoStep(backstep))
    {
        return false;
    }

    // no move in progress
    _move_ready        = false;
    _move->passive_set = false;
    _move->vector_set  = false;

    // player can make a move
    emit stepGame();
    return true;
}

// restores previously undone state
bool ShobuModel::redoStep()
{

    int step = getStepSize();

    if (!step) // can not have 0 as step
    {
        return false;
    }
    if (!_persistence->redoStep(step))
    {
        return false;
    }

    // no move in progress
    _move_ready        = false;
    _move->passive_set = false;
    _move->vector_set  = false;

    // player can make a move
    emit stepGame();
    return true;
}

// checks whether there are previous states to restore
bool ShobuModel::hasUndo()
{
    int backstep = getStepSize();

    if (!backstep) // can not have 0 as backstep
    {
        return false;
    }

    return _persistence->hasBackState(backstep);
}

// checks whether there are previously undone states to restore
bool ShobuModel::hasRedo()
{
    int step = getStepSize();

    if (!step) // can not have 0 as step
    {
        return false;
    }
    return _persistence->hasForwardState(step);
}

// Network
// connects to server of the given hostname
bool ShobuModel::connectToServer(QString hostname)
{
    return _client->connectToServer(hostname);
}

// disconnects from the server
void ShobuModel::disconnectFromServer()
{
    _client->disconnectFromServer();
}

// sends game creation request to server
void ShobuModel::createOnlineGame(GameSettings settings)
{
    newGame(settings);
    _client->createGame(settings);
}

// sends request to server for the available games
void ShobuModel::getOnlineGameList()
{
    _client->askForGames();
}

// sends request to server to join the game
void ShobuModel::joinOnlineGame(QString name)
{
    _client->joinGame(name);
}

// send draw offer to the other player
void ShobuModel::offerDraw()
{
    _draw_offer[_settings.color] = true;
    _client->offerDraw();
    emit boardChange();
}

// PRIVATE

// Game controllers
// start a game with the current state and settings
void ShobuModel::initializeGame()
{
    _persistence->initialize(); // clear all previous states, save current one

    // no move is in progress
    _move->passive_set = false;
    _move->vector_set  = false;

    _tick_count = 0;
    _move_ready = false;

    _draw_offer[WHITE] = false;
    _draw_offer[BLACK] = false;

    applySettings();

    _timer->start();
    _ticking = true;

    // first player can make a move
    emit stepGame();
}

// returns the size of step required for restoration based on game style
int ShobuModel::getStepSize()
{
    switch (_settings.style)
    {
    case SOLO:
        return _game->getTurn() == _settings.color ? 2 : 1; // we skip machine turn
        break;
    case HOTSEAT:
        return 1; // both players' turns matter
        break;
    default:
        return 0; // no step back is possible
        break;
    }
}

// prepares game with already set settings
void ShobuModel::applySettings()
{
    // time settings
    _ticking = true;

    // player settings
    if (_players[WHITE] != nullptr)
    {
        _players[WHITE]->deleteLater();
        _players[BLACK]->deleteLater();
    }

    if (_settings.style == HOTSEAT || _settings.style == NETWORK)
    {
        _players[WHITE] = new OrganicPlayer(_move, WHITE, this);
        _players[BLACK] = new OrganicPlayer(_move, BLACK, this);
    }
    else if (_settings.style == SOLO)
    {
        if (_settings.color == WHITE)
        {
            _players[WHITE] = new OrganicPlayer(_move, WHITE, this);
            _players[BLACK] = new MachinePlayer(_move, BLACK, _settings.difficulty, this);
        }
        else
        {
            _players[BLACK] = new OrganicPlayer(_move, BLACK, this);
            _players[WHITE] = new MachinePlayer(_move, WHITE, _settings.difficulty, this);
        }
    }

    // on network game client receives its player
    if (_settings.style == NETWORK)
    {
        _client->setPlayer(_players[_settings.color == WHITE ? BLACK : WHITE]);
    }

    // connectins
    QObject::connect(_players[WHITE], &ShobuPlayer::moveMade,     this, &ShobuModel::makeMove);
    QObject::connect(_players[BLACK], &ShobuPlayer::moveMade,     this, &ShobuModel::makeMove);
    QObject::connect(_players[WHITE], &ShobuPlayer::moveProgress, this, &ShobuModel::boardChange);
    QObject::connect(_players[BLACK], &ShobuPlayer::moveProgress, this, &ShobuModel::boardChange);
}

// apply the move from the MoveState
void ShobuModel::applyMove()
{
    _game->makeMove(_move->move); // apply changes


    if (_settings.style == NETWORK) // send only on online game
    {
        if (_settings.color != _game->getTurn()) // notify only when our turn is done
        {
            _client->sendMove(_move->move);
        }

        // no draw offer is in progress
        _draw_offer[WHITE] = false;
        _draw_offer[BLACK] = false;
    }
    else // save only when local game
    {
        _persistence->saveState();
    }


    // no move is in progress
    _move->passive_set = false;
    _move->vector_set  = false;

    _ticking = true; // ticking resumes in case the player paused it


    emit stepGame();
}

// Timer
// decreases remaining time if needed when _timer has timeout()
void ShobuModel::tick()
{
    if (_ticking)
    {
        if (_settings.has_time)
        {
            if (_settings.style != SOLO || _settings.color == _game->getTurn()) // the machine player is not on time
            {
                --_settings.times[_game->getTurn()];
                emit timeIsPassing();
            }

            if (_settings.style != NETWORK && _settings.times[_game->getTurn()] <= 0) // in case of network game the server watches the time
            {
                // the player whose turn is up lost the game, the other player won
                endGame();
                emit gameOver(_game->getOpponent());
            }
        }
        if (_move_ready)
        {
            if (++_tick_count >= MACHINE_INTERVAL)
            {
                _move_ready = false;
                applyMove();
            }
        }
    }
}

// Network
// starts game when server notifies us of a new starting game
void ShobuModel::onGameStarted(GameSettings settings)
{
    newGame(settings);
    emit onlineGameStarted();
}

// notifies the user of a draw offer from the other player
void ShobuModel::onDrawOffer()
{
    Color color = _settings.color == WHITE ? BLACK : WHITE; // offer came from the opponent

    _draw_offer[color] = true;
    emit boardChange();
}
