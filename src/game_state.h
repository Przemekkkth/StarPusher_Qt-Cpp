#ifndef GAME_STATE_H
#define GAME_STATE_H
#include <QPoint>
#include <QList>
struct GameState
{
    QPoint player;
    int stepCounter;
    QList<QPoint> stars;
    GameState()
        : stepCounter(0)
    {

    }
};

#endif // GAME_STATE_H
