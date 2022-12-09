#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QElapsedTimer>
#include <QPixmap>
#include <QString>
#include <QMap>
#include <QTimer>
#include <QList>
#include "level.h"

struct KeyStatus
{
    bool m_held = false;
    bool m_released = false;
};

struct MouseStatus
{
    float m_x = 0.0f;
    float m_y = 0.0f;
    bool m_released = false;
    bool m_pressed = false;
};


class GameScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit GameScene(QObject *parent = nullptr);

signals:

private slots:
    void loop();

private:
    void handlePlayerInput();
    void resetStatus();
    void readLevelsFile(QString pathFile);
    bool isWall(int x, int y);
    void floodFill(int x, int y, QChar oldChar, QChar newChar);
    void runLevel();
    void drawMap();
    bool isBlocked(int x, int y);
    bool makeMove(QString playerMoveTo);
    void decorateMap();
    bool isLevelFinished();
    void drawTilemap(QRect drawRect, const QPixmap pixmap);

    const QMap<QString, QPixmap> TILEMAPPING;
    const QMap<QString, QPixmap> OUTSIDEDECOMAPPING;
    const QList<QPixmap> PLAYERIMAGES;
    QList<Level> m_levels;
    int m_currentLevelIndex;
    int m_currentImageIndex;
    QList<QList<QChar> > m_mapObj;
    GameState m_gameStateObj;
    Level m_levelObj;
    bool m_mapNeedsRedraw;
    int m_cameraOffsetX, m_cameraOffsetY;

    KeyStatus* m_keys[256];
    MouseStatus* m_mouse;
    const int FPS = 60;
    QTimer m_timer;
    QElapsedTimer m_elapsedTimer;
    float m_deltaTime = 0.0f, m_loopTime = 0.0f;
    const float m_loopSpeed = int(1000.0f/FPS);
protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif // GAMESCENE_H
