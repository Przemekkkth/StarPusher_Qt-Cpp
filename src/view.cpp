#include "view.h"
#include <QKeyEvent>
#include <QApplication>

View::View()
    : m_gameScene(new GameScene(this)),
      m_left(false), m_right(false), m_up(false), m_down(false)
{
    setScene(m_gameScene);
    resize(m_gameScene->sceneRect().width()+2, m_gameScene->sceneRect().height()+2);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::NoFocus);
}

void View::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
    {
        QApplication::instance()->quit();
    }
    else if(event->key() == Qt::Key_Left)
    {
        m_left = false;
    }
    else if(event->key() == Qt::Key_Right)
    {
        m_right = false;
    }
    else if(event->key() == Qt::Key_Up)
    {
        m_up = false;
    }
    else if(event->key() == Qt::Key_Down)
    {
        m_down = false;
    }
    QGraphicsView::keyReleaseEvent(event);
}

void View::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Left)
    {
        m_left = true;
    }
    else if(event->key() == Qt::Key_Right)
    {
        m_right = true;
    }
    else if(event->key() == Qt::Key_Up)
    {
        m_up = true;
    }
    else if(event->key() == Qt::Key_Down)
    {
        m_down = true;
    }
    QGraphicsView::keyPressEvent(event);
}

void View::scrollContentsBy(int dx, int dy)
{
    if(!m_left && !m_right && !m_up && !m_down)
    {
        QGraphicsView::scrollContentsBy(dx, dy);
    }
}
