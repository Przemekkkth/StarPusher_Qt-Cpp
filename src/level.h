#ifndef LEVEL_H
#define LEVEL_H
#include <QList>
#include <QChar>
#include <QPoint>
#include "game_state.h"
struct Level
{
    int width;
    int height;
    QList<QList<QChar>> mapObj;
    QList<QPoint> goals;
    GameState startState;
};


#endif // LEVEL_H
