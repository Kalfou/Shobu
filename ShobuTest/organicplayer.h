#ifndef ORGANICPLAYER_H
#define ORGANICPLAYER_H

#include "shobuplayer.h"

class OrganicPlayer : public ShobuPlayer
{
    Q_OBJECT
public:
    OrganicPlayer(MoveState *m_state, Color color, QObject *parent = nullptr) : ShobuPlayer(m_state, color, parent){};

    void makeMove(int board, int row, int column) override;
    void makeMove(Move move) override;
    void makeMove() override {};
};

#endif // ORGANICPLAYER_H
