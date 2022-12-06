#include "gamescene.h"
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPixmapItem>
#include <QDebug>
#include "utils.h"
#include "pixmapmanager.h"

GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent),
      TILEMAPPING{
            {"x", PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::Corner)},
            {"#", PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::Wall)},
            {"o", PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::InsideFloor)},
            {" ", PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::OutsideFloor)}
      },
      OUTSIDEDECOMAPPING{
            {"1", PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::Rock)},
            {"2", PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::ShortTree)},
            {"3", PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::TallTree)},
            {"4", PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::UglyTree)},
      },
      PLAYERIMAGES{
            PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::Princess),
            PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::Boy),
            PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::Catgirl),
            PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::Horngirl),
            PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::Pinkgirl)
          }
{
    for(int i = 0; i < 256; ++i)
    {
        m_keys[i] = new KeyStatus();
    }
    m_mouse = new MouseStatus();
    setSceneRect(0,0, 640, 480);
    connect(&m_timer, &QTimer::timeout, this, &GameScene::loop);
    m_timer.start(int(1000.0f/FPS));
    m_elapsedTimer.start();
}

void GameScene::loop()
{
    m_deltaTime = m_elapsedTimer.elapsed();
    m_elapsedTimer.restart();

    m_loopTime += m_deltaTime;
    if( m_loopTime > m_loopSpeed)
    {
        m_loopTime -= m_loopSpeed;

        handlePlayerInput();
        resetStatus();
    }
}

void GameScene::handlePlayerInput()
{
    if(m_mouse->m_released)
    {
        qDebug() << "m_mouse->m_released " << m_mouse->m_released;
    }
}

void GameScene::resetStatus()
{
    for(int i = 0; i < 256; ++i)
    {
        m_keys[i]->m_released = false;
    }
    m_mouse->m_released = false;
}

void GameScene::keyPressEvent(QKeyEvent *event)
{
    if(KEYBOARD::KeysMapper.contains(event->key()))
    {
        m_keys[KEYBOARD::KeysMapper[event->key()]]->m_held = true;
    }
    QGraphicsScene::keyPressEvent(event);
}

void GameScene::keyReleaseEvent(QKeyEvent *event)
{
    if(KEYBOARD::KeysMapper.contains(event->key()))
    {
        m_keys[KEYBOARD::KeysMapper[event->key()]]->m_held = false;
        m_keys[KEYBOARD::KeysMapper[event->key()]]->m_released = true;
    }
    QGraphicsScene::keyReleaseEvent(event);
}

void GameScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_mouse->m_x = event->scenePos().x();
    m_mouse->m_y = event->scenePos().y();
    m_mouse->m_pressed = true;
    QGraphicsScene::mousePressEvent(event);
}

void GameScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    m_mouse->m_x = event->scenePos().x();
    m_mouse->m_y = event->scenePos().y();
    QGraphicsScene::mouseMoveEvent(event);
}

void GameScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_mouse->m_x = event->scenePos().x();
    m_mouse->m_y = event->scenePos().y();
    m_mouse->m_pressed = false;
    m_mouse->m_released = true;
    QGraphicsScene::mouseReleaseEvent(event);
}
