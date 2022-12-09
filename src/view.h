#ifndef VIEW_H
#define VIEW_H

#include <QGraphicsView>
#include "gamescene.h"

class View : public QGraphicsView
{
    Q_OBJECT
public:
    explicit View();

signals:

private:
    GameScene* m_gameScene;
    bool m_left, m_right, m_up, m_down;
    // QWidget interface
protected:
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;

    // QAbstractScrollArea interface
protected:
    virtual void scrollContentsBy(int dx, int dy) override;

};

#endif // VIEW_H
