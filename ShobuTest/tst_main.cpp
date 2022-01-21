#include <QtTest>

// add necessary includes here
#include "randomlogic.h"
#include "greedylogic.h"
#include "hardlogic.h"
#include "organicplayer.h"
#include "machineplayer.h"
#include "shobumodel.h"
#include "gamestate.h"
#include "shobuexception.h"
#include <QDebug>

class ShobuTest : public QObject // test environment
{
    Q_OBJECT

public:
    ShobuTest(){};
    ~ShobuTest(){};

private slots:
    void init();
    void cleanup();

    // GameState functions
    void initialize_blank();
    void initialize_occupied();
    void is_home_board();
    void set_state_null();
    void set_state();
    void set_field();
    void set_turn();
    void make_move();
    void end_turn();
    void get_victor();
    void is_legal_move();
    void is_legal_passive();
    void is_legal_vector();
    void is_legal_agressive();
    void get_moves();
    void get_passive_pieces();
    void get_destinations();
    void get_agressive_pieces();
    void get_applied();
    void apply_move();
    void reverse_move();

    // GameLogic children
    void random_legal();
    void greedy_legal();
    void hard_legal();
    void greedy_beats_random();
    void hard_beats_random();
    void hard_beats_greedy();
    void machine_logic_error();

    // ShobuPlayer children
    void organic_moves();
    void machine_moves();

    // ShobuModel functions
    void set_settings();
    void change_time();
    void new_game();
    void model_move(); // make_move function name is already taken
    void reset_move();
    void change_settings();
    void undo_step();
    void redo_step();
    void has_undo();
    void has_redo();

private:
    GameState *_state;

    MoveState _move;

    RandomLogic *_random;
    GreedyLogic *_greedy;
    HardLogic *_hard_white, *_hard_black;

    MachinePlayer *_machine_white, *_machine_black;
    OrganicPlayer *_organic;

    ShobuModel *_model;
};

void ShobuTest::init() // create everything
{
    _state = new GameState(this);
    _state->initializeGame();

    _move.game = _state;
    _move.passive_set = false;
    _move.vector_set  = false;

    _random = new RandomLogic(_state, this);
    _greedy = new GreedyLogic(_state, this);
    _hard_black = new HardLogic(_state, BLACK, this);
    _hard_white = new HardLogic(_state, WHITE, this);

    _machine_white = new MachinePlayer(&_move, WHITE, MEDIUM, this);
    _machine_black = new MachinePlayer(&_move, BLACK, MEDIUM, this);
    _organic = new OrganicPlayer(&_move, WHITE, this);

    _model = new ShobuModel(this);
}

void ShobuTest::cleanup() // destroy everything
{
    delete _state;
    delete _random;
    delete _greedy;
    delete _hard_black;
    delete _hard_white;
    delete _machine_white;
    delete _machine_black;
    delete _organic;
    delete _model;
}

// GameState functions

// checks the GameState::initializeGame function when the state is empty
void ShobuTest::initialize_blank()
{
    QVERIFY2(_state->getTurn() == WHITE, "White does not get the first turn");

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 1; j < 3; ++j) // check empty fields in the moddle
        {
            for (int k = 0; k < 4; ++k)
            {
                QVERIFY2(_state->getField(i,j,k) == EMPTY, "A field in the middle is not empty");
            }
        }

        for (int k = 0; k < 4; ++k) // check black pieces in first row
        {
            QVERIFY2(_state->getField(i,0,k) == BLACK, "A piece on the top is not black");
        }

        for (int k = 0; k < 4; ++k) // check white pieces in last row
        {
            QVERIFY2(_state->getField(i,3,k) == WHITE, "A piece on the bottom is not white");
        }
    }
}

// checks the GameState::initializeGame function when the state is not empty
void ShobuTest::initialize_occupied()
{
    // state is not empty
    _state->setField(1,1,1,BLACK);
    _state->setField(2,2,2,WHITE);
    _state->setTurn(BLACK);

    QVERIFY2(_state->getTurn() == BLACK, "Black does not get the turn before re-initialization");

    _state->initializeGame(); // initialize already initialized game

    QVERIFY2(_state->getTurn() == WHITE, "White does not get the first turn after re-initialization");

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 1; j < 3; ++j) // check empty fields in the middle
        {
            for (int k = 0; k < 4; ++k)
            {
                QVERIFY2(_state->getField(i,j,k) == EMPTY,  "A field in the middle is not empty");
            }
        }

        for (int k = 0; k < 4; ++k) // check black pieces in first row
        {
            QVERIFY2(_state->getField(i,0,k) == BLACK, "A piece on the top is not black");
        }

        for (int k = 0; k < 4; ++k) // check white pieces in last row
        {
            QVERIFY2(_state->getField(i,3,k) == WHITE, "A piece on the bottom is not white");
        }
    }
}

// checks the GameState::isHomeBoard function
void ShobuTest::is_home_board()
{
    // legal uses black
    QVERIFY2(GameState::isHomeBoard(BLACK, 0),  "0 should be a homeboard for black");
    QVERIFY2(GameState::isHomeBoard(BLACK, 1),  "1 should be a homeboard for black");
    QVERIFY2(!GameState::isHomeBoard(BLACK, 2), "2 should not be a homeboard for black");
    QVERIFY2(!GameState::isHomeBoard(BLACK, 3), "3 should not be a homeboard for black");

    // legal uses white
    QVERIFY2(!GameState::isHomeBoard(WHITE, 0), "0 should not be a homeboard for white");
    QVERIFY2(!GameState::isHomeBoard(WHITE, 1), "1 should not be a homeboard for white");
    QVERIFY2(GameState::isHomeBoard(WHITE, 2),  "2 should be a homeboard for white");
    QVERIFY2(GameState::isHomeBoard(WHITE, 3),  "3 should be a homeboard for white");

    // legal board, illegal color
    QVERIFY2(!GameState::isHomeBoard(EMPTY, 0), "0 should not be a homeboard for empty");
    QVERIFY2(!GameState::isHomeBoard(EMPTY, 1), "1 should not be a homeboard for empty");
    QVERIFY2(!GameState::isHomeBoard(EMPTY, 2), "2 should not be a homeboard for empty");
    QVERIFY2(!GameState::isHomeBoard(EMPTY, 3), "3 should not be a homeboard for empty");

    // illegal board
    QVERIFY2(!GameState::isHomeBoard(EMPTY, -1), "-1 should not be a homeboard");
    QVERIFY2(!GameState::isHomeBoard(WHITE, -1), "-1 should not be a homeboard");
    QVERIFY2(!GameState::isHomeBoard(BLACK, -1), "-1 should not be a homeboard");

    QVERIFY2(!GameState::isHomeBoard(EMPTY, 4), "4 should not be a homeboard");
    QVERIFY2(!GameState::isHomeBoard(WHITE, 4), "4 should not be a homeboard");
    QVERIFY2(!GameState::isHomeBoard(BLACK, 4), "4 should not be a homeboard");
}

// checks if GameState::setState throws exception when nullptr is given to it
void ShobuTest::set_state_null()
{
    GameState *null_state = nullptr;

    // cannot use null state
    QVERIFY_EXCEPTION_THROWN(_state->setState(null_state), ShobuException);
}

// checks the GameState::setState function
void ShobuTest::set_state()
{
    GameState *setter_state = new GameState(this);

    setter_state->initializeGame();
    setter_state->setField(1,1,1,BLACK);
    setter_state->setField(2,2,2,WHITE);
    setter_state->setTurn(BLACK);

    QVERIFY2(_state->getTurn() == WHITE, "Original turn is not white");
    QVERIFY2(_state->getField(1,1,1) == EMPTY, "Original middle field is not empty");
    QVERIFY2(_state->getField(2,2,2) == EMPTY, "Original middle field is not empty");

    _state->setState(setter_state);

    QVERIFY2(_state->getTurn() == BLACK, "Turn does not become black");
    QVERIFY2(_state->getField(1,1,1) == BLACK, "Field does not become black");
    QVERIFY2(_state->getField(2,2,2) == WHITE, "Field does not become white");

    setter_state->initializeGame();

    _state->setState(setter_state);
    QVERIFY2(_state->getTurn() == WHITE, "Turn does not become white");
    QVERIFY2(_state->getField(1,1,1) == EMPTY, "Field does not become empty");
    QVERIFY2(_state->getField(2,2,2) == EMPTY, "Field does not become empty");

    delete setter_state;
}

// checks the GameState::setField function
void ShobuTest::set_field()
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            for (int k = 0; k < 4; ++k) // every field, from each color to each color
            {
                _state->setField(i,j,k,EMPTY);
                QVERIFY2(_state->getField(i,j,k) == EMPTY, "One of the fields did not turn empty in the first setting");

                _state->setField(i,j,k,EMPTY);
                QVERIFY2(_state->getField(i,j,k) == EMPTY, "One of the fields did not remain empty");

                _state->setField(i,j,k,WHITE);
                QVERIFY2(_state->getField(i,j,k) == WHITE, "One of the fields did not turn white from empty");

                _state->setField(i,j,k,WHITE);
                QVERIFY2(_state->getField(i,j,k) == WHITE, "One of the fields did not remain white");

                _state->setField(i,j,k,BLACK);
                QVERIFY2(_state->getField(i,j,k) == BLACK, "One of the fields did not turn black from white");

                _state->setField(i,j,k,BLACK);
                QVERIFY2(_state->getField(i,j,k) == BLACK, "One of the fields did not remain black");

                _state->setField(i,j,k,EMPTY);
                QVERIFY2(_state->getField(i,j,k) == EMPTY, "One of the fields did not turn empty from black");

                _state->setField(i,j,k,BLACK);
                QVERIFY2(_state->getField(i,j,k) == BLACK, "One of the fields did not turn black from empty");

                _state->setField(i,j,k,WHITE);
                QVERIFY2(_state->getField(i,j,k) == WHITE, "One of the fields did not turn white from black");

                _state->setField(i,j,k,EMPTY);
                QVERIFY2(_state->getField(i,j,k) == EMPTY, "One of the fields did not turn empty from white");
            }
        }
    }
}

// checks the GameState::setTurn function
void ShobuTest::set_turn()
{
    QVERIFY2(_state->getTurn() == WHITE, "Turn is not white at the beginning");

    _state->setTurn(WHITE);
    QVERIFY2(_state->getTurn() == WHITE, "Turn did not remain white");

    _state->setTurn(BLACK);
    QVERIFY2(_state->getTurn() == BLACK, "Turn did not turn black");

    _state->setTurn(BLACK);
    QVERIFY2(_state->getTurn() == BLACK, "Turn did not remain black");

    _state->setTurn(WHITE);
    QVERIFY2(_state->getTurn() == WHITE, "Turn did not turn white");

    QVERIFY_EXCEPTION_THROWN(_state->setTurn(EMPTY), ShobuException);
}

// checks the GameState::makeMove function
void ShobuTest::make_move()
{
    Move move;

    // Peaceful step
    move.p          = Coordinate(2,3,1);
    move.row_change = -1;
    move.col_change = 1;
    move.magnitude  = 1;
    move.a          = Coordinate(1,3,2);

    _state->makeMove(move);

    // step successful
    QVERIFY2(_state->getField(2,3,1) == EMPTY, "Passive failed to leave at first step");
    QVERIFY2(_state->getField(1,3,2) == EMPTY, "Agressive failed to leave at first step");
    QVERIFY2(_state->getField(2,2,2) == WHITE, "Passive failed to arrive at first step");
    QVERIFY2(_state->getField(1,2,3) == WHITE, "Agressive failed to arrive at first step");
    QVERIFY2(_state->getTurn()       == BLACK, "First step failed to end its turn");

    // Push off board step
    move.p          = Coordinate(0,0,0);
    move.row_change = 1;
    move.col_change = 1;
    move.magnitude  = 2;
    move.a          = Coordinate(1,0,1);

    _state->makeMove(move);

    // step successful
    QVERIFY2(_state->getField(0,0,0) == EMPTY, "Passive failed to leave at second step");
    QVERIFY2(_state->getField(1,0,1) == EMPTY, "Agressive failed to leave at second step");
    QVERIFY2(_state->getField(0,2,2) == BLACK, "Passive failed to arrive at second step");
    QVERIFY2(_state->getField(1,2,3) == BLACK, "Agressive failed to arrive at second step");
    QVERIFY2(_state->getTurn()       == WHITE, "Second step failed to end its turn");

    // Illegal step
    move.p          = Coordinate(2,3,0);
    move.row_change = -1;
    move.col_change = 0;
    move.magnitude  = 2;
    move.a          = Coordinate(1,3,3);

    _state->makeMove(move);

    // illegal move is not executed
    QVERIFY2(_state->getField(2,3,0) == WHITE, "Passive piece left with an illegal move");
    QVERIFY2(_state->getField(1,3,3) == WHITE, "Agressive piece left with an illegal move");
    QVERIFY2(_state->getField(2,1,0) != WHITE, "Passive piece arrived with an illegal move");
    QVERIFY2(_state->getField(1,1,3) != WHITE, "Agressive piece arrived with an illegal move");
    QVERIFY2(_state->getTurn()       == WHITE, "An illegal move ended the turn");

    // Push step
    move.p          = Coordinate(2,3,0);
    move.row_change = -1;
    move.col_change = 0;
    move.magnitude  = 1;
    move.a          = Coordinate(1,3,3);

    _state->makeMove(move);

    // step successful
    QVERIFY2(_state->getField(2,3,0) == EMPTY, "Passive failed to leave at third step");
    QVERIFY2(_state->getField(1,3,3) == EMPTY, "Agressive failed to leave at third step");
    QVERIFY2(_state->getField(1,1,3) == BLACK, "Pushed failed to arrive at third step");
    QVERIFY2(_state->getField(2,2,0) == WHITE, "Passive failed to arrive at third step");
    QVERIFY2(_state->getField(1,2,3) == WHITE, "Agressive failed to arrive at third step");
    QVERIFY2(_state->getTurn()       == BLACK, "Third step failed to end its turn");
}

// checks the GameState::endTurn function
void ShobuTest::end_turn()
{
    QVERIFY2(_state->getTurn() == WHITE, "Original turn is not white");
    _state->endTurn();
    QVERIFY2(_state->getTurn() == BLACK, "Turn did not become black after the end of whites turn");
    _state->endTurn();
    QVERIFY2(_state->getTurn() == WHITE, "Turn did not become white after the end of blacks turn");
}

// checks the GameState::getVictor function
void ShobuTest::get_victor()
{
    QVERIFY2(_state->getVictor() == EMPTY, "Someone won the game before it started");

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            _state->setField(i,0,j, EMPTY);
        }
        QVERIFY2(_state->getVictor() == WHITE, "White did not win when blacks were entirely gone from a board");

        for (int j = 0; j < 4; ++j)
        {
            _state->setField(i,0,j, BLACK);
        }
    }

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            _state->setField(i,3,j, EMPTY);
        }
        QVERIFY2(_state->getVictor() == BLACK, "Black did not win when whites were entirely gone from a board");

        for (int j = 0; j < 4; ++j)
        {
            _state->setField(i,3,j, WHITE);
        }
    }
}

// checks the GameState::isLegalMove function
void ShobuTest::is_legal_move()
{
    Move move;

    // Illegal passives found illegal
    move.p = Coordinate(6,7,8); // very wrong
    QVERIFY2(!_state->isLegalPassive(move.p), "Coordinate off board should not be legal");

    move.p = Coordinate(0,3,3); // wrong board
    QVERIFY2(!_state->isLegalPassive(move.p), "Passive should only be allowed on a homeboard");

    move.p = Coordinate(3,0,0); // enemy field
    QVERIFY2(!_state->isLegalPassive(move.p), "Enemy pieces should not be allowed to pick as passive piece");

    move.p = Coordinate(3,2,2); // empty field
    QVERIFY2(!_state->isLegalPassive(move.p), "An empty field should not be allowed to pick as passive piece");

    for (int i = 0; i < 4; ++i) // all legal passives found legal
    {
        move.p = Coordinate(2,3,i);
        QVERIFY2(_state->isLegalPassive(move.p), "A legal passive piece was found illegal");
        move.p = Coordinate(3,3,i);
        QVERIFY2(_state->isLegalPassive(move.p), "A legal passive piece was found illegal");
    }

    move.p = Coordinate(2,3,2); // legal passive
    move.row_change = 2;
    move.col_change = 2;
    move.magnitude  = 3;

    QVERIFY2(!_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A vector with only illegal values was found legal");

    move.row_change = 1;
    move.col_change = 1;
    QVERIFY2(!_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A vector with illegal magnitude was found legal");

    move.magnitude = 0;
    QVERIFY2(!_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A vector with 0 magnitude was found legal");

    move.magnitude = 1;
    QVERIFY2(!_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A vector leading off the board was found legal");

    move.row_change = 0;
    QVERIFY2(!_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A vector pushing a friendly piece was found legal");

    move.col_change = 0;
    QVERIFY2(!_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A vector with 0 direction was found legal");

    move.row_change = -1;
    move.col_change = -1;
    QVERIFY2(_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A legal vector was found illegal");

    move.col_change = 0;
    QVERIFY2(_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A legal vector was found illegal");

    move.col_change = 1;
    QVERIFY2(_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A legal vector was found illegal");

    move.magnitude = 2;
    move.col_change = -1;
    QVERIFY2(_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A legal vector was found illegal");

    move.col_change = 0;
    QVERIFY2(_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A legal vector was found illegal");

    move.p = Coordinate(2,3,2); // legal passive
    move.row_change = -1;
    move.col_change = -1;
    move.magnitude  = 2;        // legal vector

    move.a = Coordinate(6,7,8); // very wrong
    QVERIFY2(!_state->isLegalAgressive(move), "An agressive piece shold always be on a legal field on one of the boards");

    move.a = Coordinate(3,0,0); // enemy field
    QVERIFY2(!_state->isLegalAgressive(move), "An agressive piece should not be an enemy piece");

    move.a = Coordinate(3,2,2); // empty field
    QVERIFY2(!_state->isLegalAgressive(move), "An agressive piece should not be an empty field");

    // same side board
    move.a = Coordinate(2,3,0);
    QVERIFY2(!_state->isLegalAgressive(move), "The agressive move can not be on the same board as the passive move");
    move.a = Coordinate(0,3,0);
    QVERIFY2(!_state->isLegalAgressive(move), "The agressive move can not be on the same side as the passive move");

    move.a = Coordinate(3,2,1); // destination off board
    QVERIFY2(!_state->isLegalAgressive(move), "The agressive piece should remain on the board");

    move.col_change = 0;
    move.a = Coordinate(1,3,0);
    for (int i = 0; i < 4; ++i)
    {
        move.a.column = i;
        QVERIFY2(_state->isLegalAgressive(move), "A legal agressive piece was found illegal");
    }

    move.a = Coordinate(3,3,0);
    for (int i = 0; i < 4; ++i)
    {
        move.a.column = i;
        QVERIFY2(_state->isLegalAgressive(move), "A legal agressive piece was found illegal");
    }
}

// checks the GameState::isLegalPassive function
void ShobuTest::is_legal_passive()
{
    Move move;

    // Illegal passives found illegal
    move.p = Coordinate(6,7,8); // very wrong
    QVERIFY2(!_state->isLegalPassive(move.p), "Coordinate off board should not be legal");

    move.p = Coordinate(0,3,3); // wrong board
    QVERIFY2(!_state->isLegalPassive(move.p), "Passive should only be allowed on a homeboard");

    move.p = Coordinate(3,0,0); // enemy field
    QVERIFY2(!_state->isLegalPassive(move.p), "Enemy pieces should not be allowed to pick as passive piece");

    move.p = Coordinate(3,2,2); // empty field
    QVERIFY2(!_state->isLegalPassive(move.p), "An empty field should not be allowed to pick as passive piece");

    for (int i = 0; i < 4; ++i) // all legal passives found legal
    {
        move.p = Coordinate(2,3,i);
        QVERIFY2(_state->isLegalPassive(move.p), "A legal passive piece was found illegal");
        move.p = Coordinate(3,3,i);
        QVERIFY2(_state->isLegalPassive(move.p), "A legal passive piece was found illegal");
    }
}

// checks the GameState::isLegalVector function
void ShobuTest::is_legal_vector()
{
    Move move;
    move.p = Coordinate(2,3,2); // legal passive
    move.row_change = 2;
    move.col_change = 2;
    move.magnitude  = 3;

    QVERIFY2(!_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A vector with only illegal values was found legal");

    move.row_change = 1;
    move.col_change = 1;
    QVERIFY2(!_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A vector with illegal magnitude was found legal");

    move.magnitude = 0;
    QVERIFY2(!_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A vector with 0 magnitude was found legal");

    move.magnitude = 1;
    QVERIFY2(!_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A vector leading off the board was found legal");

    move.row_change = 0;
    QVERIFY2(!_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A vector pushing a friendly piece was found legal");

    move.col_change = 0;
    QVERIFY2(!_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A vector with 0 direction was found legal");

    move.row_change = -1;
    move.col_change = -1;
    QVERIFY2(_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A legal vector was found illegal");

    move.col_change = 0;
    QVERIFY2(_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A legal vector was found illegal");

    move.col_change = 1;
    QVERIFY2(_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A legal vector was found illegal");

    move.magnitude = 2;
    move.col_change = -1;
    QVERIFY2(_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A legal vector was found illegal");

    move.col_change = 0;
    QVERIFY2(_state->isLegalVector(move.p, move.row_change, move.col_change, move.magnitude), "A legal vector was found illegal");
}

// checks the GameState::isLegalAgressive function
void ShobuTest::is_legal_agressive()
{
    Move move;
    move.p = Coordinate(2,3,2); // legal passive
    move.row_change = -1;
    move.col_change = -1;
    move.magnitude  = 2;        // legal vector

    move.a = Coordinate(6,7,8); // very wrong
    QVERIFY2(!_state->isLegalAgressive(move), "An agressive piece shold always be on a legal field on one of the boards");

    move.a = Coordinate(3,0,0); // enemy field
    QVERIFY2(!_state->isLegalAgressive(move), "An agressive piece should not be an enemy piece");

    move.a = Coordinate(3,2,2); // empty field
    QVERIFY2(!_state->isLegalAgressive(move), "An agressive piece should not be an empty field");

    // same side board
    move.a = Coordinate(2,3,0);
    QVERIFY2(!_state->isLegalAgressive(move), "The agressive move can not be on the same board as the passive move");
    move.a = Coordinate(0,3,0);
    QVERIFY2(!_state->isLegalAgressive(move), "The agressive move can not be on the same side as the passive move");

    move.a = Coordinate(3,2,1); // destination off board
    QVERIFY2(!_state->isLegalAgressive(move), "The agressive piece should remain on the board");

    move.col_change = 0;
    move.a = Coordinate(1,3,0);
    for (int i = 0; i < 4; ++i)
    {
        move.a.column = i;
        QVERIFY2(_state->isLegalAgressive(move), "A legal agressive piece was found illegal");
    }

    move.a = Coordinate(3,3,0);
    for (int i = 0; i < 4; ++i)
    {
        move.a.column = i;
        QVERIFY2(_state->isLegalAgressive(move), "A legal agressive piece was found illegal");
    }
}

// checks the GameState::getMoves function
void ShobuTest::get_moves()
{
    QVector<Move> moves = _state->getMoves();

    for (int i = 0; i < moves.length(); ++i) // all found moves are legal
    {
        QVERIFY2(_state->isLegalMove(moves[i]), "The function returned an illegal move");
    }
}

// checks the GameState::getPassivePieces function
void ShobuTest::get_passive_pieces()
{
    for (int i = 0; i < 4; ++i) // check if all passive pieces are legal on all boards
    {
        QVector<Coordinate> coords = _state->getPassivePieces(i);
        for (int j = 0; j < coords.length(); ++j)
        {
            QVERIFY2(_state->isLegalPassive(coords[j]), "The function returned an illegal passive piece");
        }
    }
}

// checks the GameState::getDestinations function
void ShobuTest::get_destinations()
{
    for (int i = 0; i < 4; ++i) // check if all passive pieces are legal on all boards
    {
        QVector<Coordinate> passives = _state->getPassivePieces(i);
        for (int j = 0; j < passives.length(); ++j)
        {
            QVector<Coordinate> destinations = _state->getDestinations(i, passives[j]);

            for (int k = 0; k < destinations.length(); ++k)
            {
                int row_change = destinations[k].row - passives[j].row;
                int col_change = destinations[k].column - passives[j].column;
                int magnitude  = 1;
                if (row_change == 2 || row_change == -2 || col_change == 2 || col_change == -2)
                {
                    magnitude = 2;
                    row_change = row_change / 2;
                    col_change = col_change / 2;
                }

                QVERIFY2(_state->isLegalVector(passives[j], row_change, col_change, magnitude), "The function returned an illegal passive destination");
            }
        }
    }
}

// checks the GameState::getAgressivePieces function
void ShobuTest::get_agressive_pieces()
{
    for (int i = 0; i < 4; ++i) // check if all passive pieces are legal on all boards
    {
        QVector<Coordinate> passives = _state->getPassivePieces(i);
        for (int j = 0; j < passives.length(); ++j)
        {
            QVector<Coordinate> destinations = _state->getDestinations(i, passives[j]);

            for (int k = 0; k < destinations.length(); ++k)
            {
                int row_change = destinations[k].row - passives[j].row;
                int col_change = destinations[k].column - passives[j].column;
                int magnitude  = 1;
                if (row_change == 2 || row_change == -2 || col_change == 2 || col_change == -2)
                {
                    magnitude = 2;
                    row_change = row_change / 2;
                    col_change = col_change / 2;
                }

                QVector<Coordinate> agressives = _state->getAgressivePieces(i, passives[j], row_change, col_change, magnitude);

                for (int l = 0; l < agressives.length(); ++l)
                {
                    Move move;
                    move.p          = passives[j];
                    move.row_change = row_change;
                    move.col_change = col_change;
                    move.magnitude  = magnitude;
                    move.a          = agressives[l];

                    QVERIFY2(_state->isLegalAgressive(move), "The function returned an illegal agressive piece");
                }
            }
        }
    }
}

// checks the GameState::getApplied function
void ShobuTest::get_applied()
{
    QVector<Move> moves = _state->getMoves();

    for (int i  = 0; i < moves.length(); ++i)
    {
        GameState *applied = _state->getApplied(moves[i]);
        ReverseData reverse = _state->applyMove(moves[i]);

        QVERIFY2(_state->getTurn() == applied->getTurn(), "The returned GameState's turn does not match the normally applied one");

        for (int j = 0; j < 4; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                for (int l = 0; l < 4; ++l)
                {
                    QVERIFY2(_state->getField(j,k,l) == applied->getField(j,k,l), "One of the fields of the returned GameState does not match the original");
                }
            }
        }

        delete applied;
        _state->reverseMove(moves[i], reverse);
    }
}

// checks the GameState::applyMove function
void ShobuTest::apply_move()
{
    Move move;

    // Peaceful step
    move.p          = Coordinate(2,3,1);
    move.row_change = -1;
    move.col_change = 1;
    move.magnitude  = 1;
    move.a          = Coordinate(1,3,2);

    _state->applyMove(move);

    // step successful
    QVERIFY2(_state->getField(2,3,1) == EMPTY, "Passive failed to leave at first step");
    QVERIFY2(_state->getField(1,3,2) == EMPTY, "Agressive failed to leave at first step");
    QVERIFY2(_state->getField(2,2,2) == WHITE, "Passive failed to arrive at first step");
    QVERIFY2(_state->getField(1,2,3) == WHITE, "Agressive failed to arrive at first step");
    QVERIFY2(_state->getTurn()       == BLACK, "First step failed to end its turn");

    // Push off board step
    move.p          = Coordinate(0,0,0);
    move.row_change = 1;
    move.col_change = 1;
    move.magnitude  = 2;
    move.a          = Coordinate(1,0,1);

    _state->applyMove(move);

    // step successful
    QVERIFY2(_state->getField(0,0,0) == EMPTY, "Passive failed to leave at second step");
    QVERIFY2(_state->getField(1,0,1) == EMPTY, "Agressive failed to leave at second step");
    QVERIFY2(_state->getField(0,2,2) == BLACK, "Passive failed to arrive at second step");
    QVERIFY2(_state->getField(1,2,3) == BLACK, "Agressive failed to arrive at second step");
    QVERIFY2(_state->getTurn()       == WHITE, "Second step failed to end its turn");

    // Illegal step
    move.p          = Coordinate(2,3,0);
    move.row_change = -1;
    move.col_change = 0;
    move.magnitude  = 2;
    move.a          = Coordinate(1,3,3);

    QVERIFY_EXCEPTION_THROWN(_state->applyMove(move), ShobuException);

    // Push step
    move.p          = Coordinate(2,3,0);
    move.row_change = -1;
    move.col_change = 0;
    move.magnitude  = 1;
    move.a          = Coordinate(1,3,3);

    _state->applyMove(move);

    // step successful
    QVERIFY2(_state->getField(2,3,0) == EMPTY, "Passive failed to leave at third step");
    QVERIFY2(_state->getField(1,3,3) == EMPTY, "Agressive failed to leave at third step");
    QVERIFY2(_state->getField(1,1,3) == BLACK, "Pushed failed to arrive at third step");
    QVERIFY2(_state->getField(2,2,0) == WHITE, "Passive failed to arrive at third step");
    QVERIFY2(_state->getField(1,2,3) == WHITE, "Agressive failed to arrive at third step");
    QVERIFY2(_state->getTurn()       == BLACK, "Third step failed to end its turn");
}

// checks the GameState::reverseMove function
void ShobuTest::reverse_move()
{
    QVector<Move> moves = _state->getMoves();

    for (int i  = 0; i < moves.length(); ++i)
    {
        GameState *applied = new GameState(this);
        applied->setState(_state);
        ReverseData reverse = applied->applyMove(moves[i]);
        applied->reverseMove(moves[i], reverse);

        QVERIFY2(_state->getTurn() == applied->getTurn(), "The turn did not change back to the previous color");

        for (int j = 0; j < 4; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                for (int l = 0; l < 4; ++l)
                {
                    QVERIFY2(_state->getField(j,k,l) == applied->getField(j,k,l), "One of the fields does not match the previous state after reversing");
                }
            }
        }
        delete applied;
    }
}

// GameLogic children

// checks the RandomLogic::getMove function
void ShobuTest::random_legal()
{
    for (int i = 10; i < 8; ++i)
    {
        Move move = _random->getMove();
        QVERIFY2(_state->isLegalMove(move), "Returned move is illegal");
        _state->applyMove(move);
    }
}

// checks the GreedyLogic::getMove function
void ShobuTest::greedy_legal()
{
    for (int i = 10; i < 8; ++i)
    {
        Move move = _greedy->getMove();
        QVERIFY2(_state->isLegalMove(move), "Returned move is illegal");
        _state->applyMove(move);
    }
}

// checks the HardLogic::getMove function
void ShobuTest::hard_legal()
{
    HardLogic *_hard[2];
    _hard[WHITE] = _hard_white;
    _hard[BLACK] = _hard_black;

    for (int i = 10; i < 8; ++i)
    {
        Move move = _hard[_state->getTurn()]->getMove();
        QVERIFY2(_state->isLegalMove(move), "Returned move is illegal");
        _state->applyMove(move);
    }
}

// checks if the greedy logic can beat the random logic
void ShobuTest::greedy_beats_random()
{
    MachineLogic *_logics[2];

    _logics[BLACK] = _greedy;
    _logics[WHITE] = _random;

    for (int i = 0; i < 10; ++i) // win 10 game in a row
    {
        _state->initializeGame();
        while (_state->getVictor() == EMPTY)
        {
            Move move = _logics[_state->getTurn()]->getMove();
            QVERIFY2(_state->isLegalMove(move), "One of the logics attempted an illegal move");
            _state->applyMove(move);
        }

        QVERIFY2(_state->getVictor() == BLACK, "The random logic won against the greedy logic. It is unlikely, but not entirely impossible");
    }
}

// checks if the hard logic can beat the random logic
void ShobuTest::hard_beats_random()
{
    qDebug()<<"There is a small chance of this failing because of the random variable.";

    MachineLogic *_logics[2];

    _logics[BLACK] = _hard_black;
    _logics[WHITE] = _random;

    for (int i = 0; i < 10; ++i)  // win 10 game in a row
    {
        _state->initializeGame();

        while (_state->getVictor() == EMPTY)
        {
            Move move = _logics[_state->getTurn()]->getMove();
            QVERIFY2(_state->isLegalMove(move), "One of the logics attempted an illegal move");
            _state->applyMove(move);
        }

        QVERIFY2(_state->getVictor() == BLACK, "The random logic won against the hard logic. It is unlikely, but not entirely impossible");
    }
}

// checks if the hard logic can beat the greedy logic
void ShobuTest::hard_beats_greedy()
{
    qDebug()<<"There is a small chance of this failing because of the random variable.";

    MachineLogic *_logics[2];

    _logics[BLACK] = _greedy;
    _logics[WHITE] = _hard_white;

    int victory = 0;

    for (int i = 0; i < 100; ++i)  // 10 games in a row
    {
        _state->initializeGame();

        int stepcount = 0;
        while (_state->getVictor() == EMPTY && stepcount++ < 10)
        {
            Move move = _logics[_state->getTurn()]->getMove();
            QVERIFY2(_state->isLegalMove(move), "One of the logics attempted an illegal move");
            _state->applyMove(move);
        }
        if (_state->getVictor() == WHITE)
        {
            ++victory;
        }
    }

    // hardlogic wins at least 70% of times (random element of greedy makes victory uncertain)
    QVERIFY2(victory >= 7, "The greedy logic won against the hard logic, or evaded it for more than 200 rounds more than three times."
                           "It is unlikely, but not entirely impossible");
}

// check if MachinLogic::getMove throws an error if no legal moves are available
void ShobuTest::machine_logic_error()
{
    for (int i = 0; i < 4; ++i) // block all passive moves
    {
        _state->setField(2,2,i,BLACK);
        _state->setField(3,2,i,BLACK);
    }

    // throws exception when no moves are available
    QVERIFY_EXCEPTION_THROWN(_random->getMove(), ShobuException);
    QVERIFY_EXCEPTION_THROWN(_greedy->getMove(), ShobuException);
    QVERIFY_EXCEPTION_THROWN(_hard_white->getMove(), ShobuException);
}

// ShobuPlayer children

// checks the OrganicPlayer::makeMove function
void ShobuTest::organic_moves()
{
    Move move;

    // Illegal passives found illegal
    _organic->makeMove(6,7,8); // very wrong
    QVERIFY2(!_move.passive_set, "The function should not accept a coordinate off the board as a passive piece");

    _organic->makeMove(0,3,3); // wrong board
    QVERIFY2(!_move.passive_set, "The passive piece should be from a homeboard");

    _organic->makeMove(3,0,0); // enemy field
    QVERIFY2(!_move.passive_set, "The passive piece can not be an enemy piece");

    _organic->makeMove(3,2,2); // empty field
    QVERIFY2(!_move.passive_set, "An empty field can not be a passive piece");

    _organic->makeMove(2,3,2); // legal passive
    QVERIFY2(_move.passive_set, "A legal passive piece was found illegal");

    move.p = Coordinate(2,3,2);

    // Illegal vectors found illegal
    QVERIFY2(!_move.vector_set, "Vector should not be set before it is set by the user");

    _organic->makeMove(5,-2,9999999); // very wrong
    QVERIFY2(!_move.vector_set, "A vector should not accept parameters that are out of range");

    _organic->makeMove(1,2,3); // wrong board
    QVERIFY2(!_move.vector_set, "The passive destination must be on the same board as the passive piece");

    _organic->makeMove(2,-2,5); // off board
    QVERIFY2(!_move.vector_set, "The passive destination can not bo off the board");

    _organic->makeMove(3,4,5); // off board
    QVERIFY2(!_move.vector_set, "The passive destination can not bo off the board");

    _organic->makeMove(2,3,2); // passive selected again
    QVERIFY2(!_move.vector_set, "The second legal passive piece was found illegal");

    _organic->makeMove(2,3,3); // self pushing
    QVERIFY2(!_move.vector_set, "A passive piece can not push one of its allies");

    _organic->makeMove(2,1,1); // different magnitude on row and col
    QVERIFY2(!_move.vector_set, "The magnitude of the change must be the same on rows and columns");

    _organic->makeMove(2,1,0); // good
    QVERIFY2(_move.vector_set, "A legal vector was found illegal");
}

// checks the MachineLogic::makeMove function
void ShobuTest::machine_moves()
{
    MachinePlayer *machine[2];
    machine[WHITE] = _machine_white;
    machine[BLACK] = _machine_black;

    for (int i = 10; i < 8; ++i)
    {
        machine[_state->getTurn()]->makeMove();
        QVERIFY2(_state->isLegalMove(_move.move), "The machine logic attempted an illegal move");
        _state->applyMove(_move.move);
    }

    for (int i = 0; i < 4; ++i) // block all passive moves
    {
        _state->setField(2,2,i,BLACK);
        _state->setField(3,2,i,BLACK);
    }

    QVERIFY_EXCEPTION_THROWN(machine[WHITE]->makeMove(), ShobuException);
}

// ShobuModel functions

// checks the ShobuModel::setSettings function
void ShobuTest::set_settings()
{
    GameSettings settings;
    settings.style = HOTSEAT;
    settings.has_time = true;
    settings.time = 100;

    _model->setSettings(settings);
    QVERIFY2(settings.style    == _model->getSettings().style,    "Style did not change");
    QVERIFY2(settings.has_time == _model->getSettings().has_time, "Time limit did not change");
    QVERIFY2(settings.time     == _model->getSettings().time,     "Time did not change");

    settings.style = SOLO;
    settings.difficulty = MEDIUM;
    settings.has_time = false;

    QVERIFY2(settings.style    != _model->getSettings().style,    "Style did not change the second time");
    QVERIFY2(settings.has_time != _model->getSettings().has_time, "Time limit did not change the second time");

    _model->setSettings(settings);

    QVERIFY2(settings.style      == _model->getSettings().style,      "Style did not change the third time");
    QVERIFY2(settings.has_time   == _model->getSettings().has_time,   "Time limit did not change the third time");
    QVERIFY2(settings.difficulty == _model->getSettings().difficulty, "Difficulty did not change the third time");
}

// checks the ShobuModel::changeTime function
void ShobuTest::change_time()
{
    GameSettings settings;
    settings.style = HOTSEAT;
    settings.has_time = false;

    _model->newGame(settings);

    QVERIFY2(_model->getTicking(),  "Timing did not start at the beginning of the game");

    _model->changeTime();

    QVERIFY2(!_model->getTicking(), "Timing did not stop after stopping it");

    _model->changeTime();

    QVERIFY2(_model->getTicking(),  "Timing did not resume after resterting it");
}

// checks the ShobuModel::newGame function
void ShobuTest::new_game()
{
    GameSettings settings;
    settings.style = HOTSEAT;
    settings.has_time = false;

    _model->newGame(settings);

    QVERIFY2(_state->getTurn() == WHITE, "White did not get the first turn");

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 1; j < 3; ++j) // check empty fields in the moddle
        {
            for (int k = 0; k < 4; ++k)
            {
                QVERIFY2(_state->getField(i,j,k) == EMPTY, "One of the fields in the middle was not empty");
            }
        }

        for (int k = 0; k < 4; ++k) // check black pieces in first row
        {
            QVERIFY2(_state->getField(i,0,k) == BLACK, "One of the fields on the top was not black");
        }

        for (int k = 0; k < 4; ++k) // check white pieces in last row
        {
            QVERIFY2(_state->getField(i,3,k) == WHITE, "One of the fields on the bottom was not white");
        }
    }

    QVERIFY2(_model->getTicking(), "Timing did not start after the game started");
}

// checks the ShobuModel::makeMove function
void ShobuTest::model_move()
{
    GameSettings settings;
    settings.style = HOTSEAT;
    settings.has_time = false;

    _model->newGame(settings);

    Move move;
    move.p = Coordinate(3,3,3);
    move.row_change = -1;
    move.col_change = 0;
    move.magnitude  = 1;
    move.a = Coordinate(0,3,1);

    // model makes move after white player signals
    _model->getPlayer(WHITE)->makeMove(move);

    QVERIFY2(_model->getMoveState()->game->getField(3,3,3) == EMPTY, "Passive did not leave its field at the first step");
    QVERIFY2(_model->getMoveState()->game->getField(0,3,1) == EMPTY, "Agressive did not leave its field at the first step");
    QVERIFY2(_model->getMoveState()->game->getField(3,2,3) == WHITE, "Passive did not arrive to its field at the first step");
    QVERIFY2(_model->getMoveState()->game->getField(0,2,1) == WHITE, "Agressive did not arrive to its field at the first step");
    QVERIFY2(_model->getMoveState()->game->getTurn()       == BLACK, "The first step did not end the turn");

    move.p = Coordinate(0,0,0);
    move.row_change = 1;
    move.col_change = 1;
    move.magnitude  = 1;
    move.a = Coordinate(3,0,1);

    // model makes move after black player signals
    _model->getPlayer(BLACK)->makeMove(move);

    QVERIFY2(_model->getMoveState()->game->getField(0,0,0) == EMPTY, "Passive did not leave its field at the second step");
    QVERIFY2(_model->getMoveState()->game->getField(3,0,1) == EMPTY, "Agressive did not leave its field at the second step");
    QVERIFY2(_model->getMoveState()->game->getField(0,1,1) == BLACK, "Passive did not arrive to its field at the second step");
    QVERIFY2(_model->getMoveState()->game->getField(3,1,2) == BLACK, "Agressive did not arrive to its field at the second step");
    QVERIFY2(_model->getMoveState()->game->getTurn()       == WHITE, "The second step did not end the turn");
}

// checks the ShobuModel::resetMove function
void ShobuTest::reset_move()
{
    QVERIFY2(!_model->getMoveState()->passive_set, "Passive piece should not be set before the uset has set it");
    QVERIFY2(!_model->getMoveState()->vector_set,  "Vector should not be set before the uset has set it");

    _model->getMoveState()->passive_set = true;
    QVERIFY2(_model->getMoveState()->passive_set, "Passive should be set after setting it");
    QVERIFY2(!_model->getMoveState()->vector_set, "Vector should not be set when only the passive is set");

    _model->resetMove();

    QVERIFY2(!_model->getMoveState()->passive_set, "Passive should not be set after resetting");
    QVERIFY2(!_model->getMoveState()->vector_set,  "Vector should not be set after resetting");

    _model->getMoveState()->passive_set = true;
    QVERIFY2(_model->getMoveState()->passive_set, "Passive should be set after setting it");
    QVERIFY2(!_model->getMoveState()->vector_set, "Vector should not be set when only the passive is set");

    _model->getMoveState()->vector_set = true;
    QVERIFY2(_model->getMoveState()->passive_set, "Passive should be set after setting it");
    QVERIFY2(_model->getMoveState()->vector_set,  "Vector should be set after setting it");

    _model->resetMove();

    QVERIFY2(!_model->getMoveState()->passive_set, "Passive should not be set after resetting");
    QVERIFY2(!_model->getMoveState()->vector_set,  "Vector should not be set after resetting");
}

// checks the ShobuModel::changeSettings function
void ShobuTest::change_settings()
{
    GameSettings settings;
    settings.style = HOTSEAT;
    settings.has_time = true;
    settings.time = 100;

    _model->changeSettings(settings);
    QVERIFY2(settings.style    == _model->getSettings().style,    "Style did not change");
    QVERIFY2(settings.has_time == _model->getSettings().has_time, "Time limit did not change");
    QVERIFY2(settings.time     == _model->getSettings().time,     "Time did not change");

    settings.style = SOLO;
    settings.difficulty = MEDIUM;
    settings.has_time = false;

    QVERIFY2(settings.style    != _model->getSettings().style,    "Style did not change the second time");
    QVERIFY2(settings.has_time != _model->getSettings().has_time, "Time limit did not change the second time");

    _model->changeSettings(settings);

    QVERIFY2(settings.style      == _model->getSettings().style,      "Style did not change the third time");
    QVERIFY2(settings.has_time   == _model->getSettings().has_time,   "Time limit did not change the third time");
    QVERIFY2(settings.difficulty == _model->getSettings().difficulty, "Difficulty did not change the third time");
}

// checks the ShobuModel::undoStep function
void ShobuTest::undo_step()
{
    GameState *copy_state = new GameState(this);

    GameSettings settings;
    settings.style = HOTSEAT;
    settings.has_time = false;

    _model->newGame(settings);

    Move move;
    move.p = Coordinate(3,3,3);
    move.row_change = -1;
    move.col_change = 0;
    move.magnitude  = 1;
    move.a = Coordinate(0,3,1);

    copy_state->setState(_model->getMoveState()->game);

    _model->getPlayer(WHITE)->makeMove(move);

    _model->undoStep();

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                QVERIFY2(copy_state->getField(i,j,k) == _model->getMoveState()->game->getField(i,j,k),
                         "One of the fields did not revert to the previous state");
            }
        }
    }
    QVERIFY2(copy_state->getTurn() == _model->getMoveState()->game->getTurn(), "The turn did not revert to the previous state");

    _model->getPlayer(WHITE)->makeMove(move);

    copy_state->setState(_model->getMoveState()->game);

    move.p = Coordinate(0,0,0);
    move.row_change = 1;
    move.col_change = 1;
    move.magnitude  = 1;
    move.a = Coordinate(3,0,1);

    // model makes move after black player signals
    _model->getPlayer(BLACK)->makeMove(move);

    _model->undoStep();

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                QVERIFY2(copy_state->getField(i,j,k) == _model->getMoveState()->game->getField(i,j,k),
                         "One of the fields did not revert to the previous state");
            }
        }
    }
    QVERIFY2(copy_state->getTurn() == _model->getMoveState()->game->getTurn(), "The turn did not revert to the previous state");

    delete copy_state;
}

// checks the ShobuModel::redoStep function
void ShobuTest::redo_step()
{
    GameState *copy_state = new GameState(this);

    GameSettings settings;
    settings.style = HOTSEAT;
    settings.has_time = false;

    _model->newGame(settings);

    Move move;
    move.p = Coordinate(3,3,3);
    move.row_change = -1;
    move.col_change = 0;
    move.magnitude  = 1;
    move.a = Coordinate(0,3,1);

    _model->getPlayer(WHITE)->makeMove(move);

    copy_state->setState(_model->getMoveState()->game);
    _model->undoStep();
    _model->redoStep();

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            for (int k = 0; k < 4; ++k) // compare every field
            {
                QVERIFY2(copy_state->getField(i,j,k) == _model->getMoveState()->game->getField(i,j,k), "One of the fields did not redo the undone step correctly");
            }
        }
    }
    QVERIFY2(copy_state->getTurn() == _model->getMoveState()->game->getTurn(), "The turn did not change back after redoing a step");

    move.p = Coordinate(0,0,0);
    move.row_change = 1;
    move.col_change = 1;
    move.magnitude  = 1;
    move.a = Coordinate(3,0,1);

    // model makes move after black player signals
    _model->getPlayer(BLACK)->makeMove(move);

    copy_state->setState(_model->getMoveState()->game);

    _model->undoStep();

    _model->redoStep();

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            for (int k = 0; k < 4; ++k) // every field, from each color to each color
            {
                QVERIFY2(copy_state->getField(i,j,k) == _model->getMoveState()->game->getField(i,j,k), "One of the fields did not redo the undone step correctly");
            }
        }
    }
    QVERIFY2(copy_state->getTurn() == _model->getMoveState()->game->getTurn(), "The turn did not change back after redoing a step");

    delete copy_state;
}

// checks the ShobuModel::hasUndo function
void ShobuTest::has_undo()
{

    GameSettings settings;
    settings.style = HOTSEAT;
    settings.has_time = false;

    _model->newGame(settings);

    Move move;
    move.p = Coordinate(3,3,3);
    move.row_change = -1;
    move.col_change = 0;
    move.magnitude  = 1;
    move.a = Coordinate(0,3,1);

    QVERIFY2(!_model->hasUndo(), "Undo should not be possible at the first turn");
    _model->getPlayer(WHITE)->makeMove(move);

    QVERIFY2(_model->hasUndo(), "Undo should  be possible after the first move");

    _model->undoStep();

    QVERIFY2(!_model->hasUndo(), "Undo should not be possible after undoing the only move");


    _model->getPlayer(WHITE)->makeMove(move);

    QVERIFY2(_model->hasUndo(), "Undo should  be possible after making the first move the second time");

    move.p = Coordinate(0,0,0);
    move.row_change = 1;
    move.col_change = 1;
    move.magnitude  = 1;
    move.a = Coordinate(3,0,1);

    // model makes move after black player signals
    _model->getPlayer(BLACK)->makeMove(move);

    QVERIFY2(_model->hasUndo(), "Undo should be possible after the second move");

    _model->undoStep();

    QVERIFY2(_model->hasUndo(), "Undo should be possible after undoing only one of the two moves");
}

// checks the ShobuModel::hasUndo function
void ShobuTest::has_redo()
{
    GameSettings settings;
    settings.style = HOTSEAT;
    settings.has_time = false;

    _model->newGame(settings);

    Move move;
    move.p = Coordinate(3,3,3);
    move.row_change = -1;
    move.col_change = 0;
    move.magnitude  = 1;
    move.a = Coordinate(0,3,1);

    QVERIFY2(!_model->hasRedo(), "Redo should not be possible at the first turn");
    _model->getPlayer(WHITE)->makeMove(move);

    QVERIFY2(!_model->hasRedo(), "Redo should not be possible after the first move");

    _model->undoStep();

    QVERIFY2(_model->hasRedo(), "Redo should be possible after undoing the first step");

    _model->getPlayer(WHITE)->makeMove(move);

    QVERIFY2(!_model->hasRedo(), "Redo should not be possible after redoing the first step");

    move.p = Coordinate(0,0,0);
    move.row_change = 1;
    move.col_change = 1;
    move.magnitude  = 1;
    move.a = Coordinate(3,0,1);

    // model makes move after black player signals
    _model->getPlayer(BLACK)->makeMove(move);

    QVERIFY2(!_model->hasRedo(), "Redo should not be possible after the second step");

    _model->undoStep();

    QVERIFY2(_model->hasRedo(), "Redo should be possible after undoing the second step");

    _model->undoStep();

    QVERIFY2(_model->hasRedo(), "Redo should be possible after undoing the first step");

    _model->redoStep();

    QVERIFY2(_model->hasRedo(), "Redo should be possible after redoing the first step, but not the second");
}

QTEST_APPLESS_MAIN(ShobuTest)

#include "tst_main.moc"
