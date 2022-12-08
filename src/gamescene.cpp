#include "gamescene.h"
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include "utils.h"
#include "pixmapmanager.h"
#include "game_state.h"

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
          },
      m_currentLevelIndex(0),
      m_currentImageIndex(0)
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
    srand(time(0));
    readLevelsFile(":/res/lvl/test.txt");
    runLevel();
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

void GameScene::readLevelsFile(QString pathFile)
{
    int maxWidth = -1;
    QStringList dataContent;
    QFile file(pathFile);
    if(file.open(QIODevice::ReadOnly))
    {
        qDebug() << "file is open";
        QTextStream stream(&file);
        while(!stream.atEnd())
        {
            QString line = stream.readLine();
            dataContent.push_back(line);
            //qDebug() << line << " length " << line.length();
        }
        //Find the longest row in the map
        for(int i = 0; i < dataContent.size(); ++i)
        {
            if(dataContent[i].length() > maxWidth)
            {
                maxWidth = dataContent[i].length();
            }
        }
        //Be sure map is rectange
        for(int i = 0; i < dataContent.size(); ++i)
        {
            if(dataContent[i].length() < maxWidth)
            {
                for(int j = 0; j <= maxWidth-dataContent[i].length(); ++j)
                {
                    dataContent[i] += " ";
                }
            }
        }
        //Resize and fill map object
        QList<QList<QChar> > mapObj;
        mapObj.resize(dataContent.size());
        for(int y = 0; y < dataContent.size(); ++y)
        {
            for(int x = 0; x < maxWidth; ++x)
            {
                mapObj[x].push_back(dataContent[y][x]);
            }
        }
        //Loop through the spaces in the map and find the @, ., and $
        //characters for the starting game state.

        QPoint startPos;
        QList<QPoint> goals;
        QList<QPoint> stars;
        for(int x = 0; x < maxWidth; ++x)
        {
            for(int y = 0; y < mapObj[x].length(); ++y)
            {
                if(mapObj[x][y] == QChar('@') || mapObj[x][y] == QChar('+'))
                {
                    // '@' is player, '+' is player & goal
                    startPos.setX(x);
                    startPos.setY(y);
                }
                if(mapObj[x][y] == QChar('.') || mapObj[x][y] == QChar('+') || mapObj[x][y] == QChar('*'))
                {
                    // '.' is goal, '*' is star & goal
                    QPoint p(x,y);
                    goals.push_back(p);
                }
                if(mapObj[x][y] == QChar('$') || mapObj[x][y] == QChar('*'))
                {
                    // '$' is star
                    QPoint p(x,y);
                    stars.push_back(p);
                }
            }
        }
        ////////////
        GameState gameStateObj;
        gameStateObj.player = startPos;
        gameStateObj.stepCounter = 0;
        gameStateObj.stars = stars;

        Level levelObj;
        levelObj.width  = maxWidth;
        levelObj.height = dataContent.length();
        levelObj.mapObj = mapObj;
        levelObj.goals  = goals;
        levelObj.startState = gameStateObj;

        m_levels.push_back(levelObj);
    }
    else{
        qDebug() << "file is not open";
    }
    file.close();
}

bool GameScene::isWall(QList<QList<QChar> > mapObj, int x, int y)
{
    if(x < 0 || x >= mapObj.length() || y < 0 || y >= mapObj[x].length())
    {
        return false;
    }
    else if(mapObj[x][y] == QChar('#') || mapObj[x][y] == QChar('x'))
    {
        return true;
    }
    return false;
}

void GameScene::floodFill(QList<QList<QChar> > &mapObj, int x, int y, QChar oldChar, QChar newChar)
{
    if(mapObj[x][y] == oldChar)
    {
        mapObj[x][y] = newChar;
    }

    if(x < mapObj.length()-1 && mapObj[x+1][y] == oldChar)
    {
        floodFill(mapObj, x+1, y, oldChar, newChar); // call right
    }
    if(x > 0 && mapObj[x-1][y] == oldChar)
    {
        floodFill(mapObj, x-1, y, oldChar, newChar); // call left
    }
    if(y < mapObj[x].length() && mapObj[x][y+1] == oldChar)
    {
        floodFill(mapObj, x, y+1, oldChar, newChar); // call down
    }
    if(y > 0 && mapObj[x][y-1] == oldChar)
    {
        floodFill(mapObj, x, y-1, oldChar, newChar); // call up
    }
}

void GameScene::runLevel()
{
    Level levelObj = m_levels[m_currentLevelIndex];
    QList mapObj = decorateMap(levelObj.mapObj, levelObj.startState.player);
    GameState gameStateObj = levelObj.startState;
    int mapWidth = mapObj.length() * GAME::TILEWIDTH;
    int mapHeight = (mapObj[0].length() - 1) * GAME::TILEFLOORHEIGHT + GAME::TILEHEIGHT;
    drawMap(mapObj, levelObj.startState, levelObj.goals);
}

void GameScene::drawMap(QList<QList<QChar> > &mapObj, GameState gameState, QList<QPoint> goals)
{
    int mapSurfWidth = mapObj.length() * GAME::TILEWIDTH;
    int mapSurfHeight = (mapObj[0].length()-1) * GAME::TILEFLOORHEIGHT + GAME::TILEHEIGHT;
    setBackgroundBrush(GAME::BGCOLOR);
    for(int x = 0; x < mapObj.length(); ++x)
    {
        for(int y = 0; y < mapObj[x].length(); ++y)
        {
            QRect spaceRect = QRect(x*GAME::TILEWIDTH, y*GAME::TILEFLOORHEIGHT,
                                    GAME::TILEWIDTH, GAME::TILEHEIGHT);
            QPixmap pixmap;
            bool isDraw = false;
            if(TILEMAPPING.contains(QString(mapObj[x][y])))
            {
                //baseTile = TILEMAPPING[mapObj[x][y]]
                pixmap = TILEMAPPING[QString(mapObj[x][y])];
                isDraw = true;
            }
            else if(OUTSIDEDECOMAPPING.contains(QString(mapObj[x][y])))
            {
                pixmap = TILEMAPPING[" "];
                isDraw = true;
            }
            if(isDraw)
            {
                QGraphicsPixmapItem* pItem = new QGraphicsPixmapItem();
                pItem->setPos(spaceRect.x(), spaceRect.y());
                pItem->setPixmap(pixmap.scaled(spaceRect.width(), spaceRect.height()));
                addItem(pItem);
            }

            if(OUTSIDEDECOMAPPING.contains(QString(mapObj[x][y])))
            {
                pixmap = OUTSIDEDECOMAPPING[QString(mapObj[x][y])];
                QGraphicsPixmapItem* pItem = new QGraphicsPixmapItem();
                pItem->setPos(spaceRect.x(), spaceRect.y());
                pItem->setPixmap(pixmap.scaled(spaceRect.width(), spaceRect.height()));
                addItem(pItem);
            }
            else if(gameState.stars.contains(QPoint(x,y)))
            {
                if(goals.contains(QPoint(x,y)))
                {
                    pixmap = PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::CoveredGoal);
                    QGraphicsPixmapItem* pItem = new QGraphicsPixmapItem();
                    pItem->setPos(spaceRect.x(), spaceRect.y());
                    pItem->setPixmap(pixmap.scaled(spaceRect.width(), spaceRect.height()));
                    addItem(pItem);
                }
                pixmap = PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::Star);
                QGraphicsPixmapItem* pItem = new QGraphicsPixmapItem();
                pItem->setPos(spaceRect.x(), spaceRect.y());
                pItem->setPixmap(pixmap.scaled(spaceRect.width(), spaceRect.height()));
                addItem(pItem);
            }
            else if(goals.contains(QPoint(x,y)))
            {
                pixmap = PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::CoveredGoal);
                QGraphicsPixmapItem* pItem = new QGraphicsPixmapItem();
                pItem->setPos(spaceRect.x(), spaceRect.y());
                pItem->setPixmap(pixmap.scaled(spaceRect.width(), spaceRect.height()));
                addItem(pItem);
            }

            if(gameState.player == QPoint(x,y))
            {
                pixmap = PLAYERIMAGES.at(m_currentImageIndex);
                QGraphicsPixmapItem* pItem = new QGraphicsPixmapItem();
                pItem->setPos(spaceRect.x(), spaceRect.y());
                pItem->setPixmap(pixmap.scaled(spaceRect.width(), spaceRect.height()));
                addItem(pItem);
            }
        }
    }
}

QList<QList<QChar> > GameScene::decorateMap(QList<QList<QChar> > mapObj, QPoint startPos)
{
    // Remove the non-wall characters from the map data
    QList<QList<QChar> > mapCopyObj = mapObj;
    for(int x = 0; x < mapCopyObj.length(); ++x)
    {
        for(int y = 0; y < mapCopyObj[0].length(); ++y)
        {
            if(     mapCopyObj[x][y] == QChar('$') ||
                    mapCopyObj[x][y] == QChar('.') ||
                    mapCopyObj[x][y] == QChar('@') ||
                    mapCopyObj[x][y] == QChar('+') ||
                    mapCopyObj[x][y] == QChar('*') )
            {
                mapCopyObj[x][y] = QChar(' ');
            }
        }
    }
    // Flood fill to determine inside/outside floor tiles.
    floodFill(mapCopyObj, startPos.x(), startPos.y(), ' ', 'o');
    //Convert the adjoined walls into corner tiles.
    for(int x = 0; x < mapCopyObj.length(); ++x)
    {
        for(int y = 0; y < mapCopyObj[0].length(); ++y)
        {
            if(mapCopyObj[x][y] == QChar('#') )
            {
                if( (isWall(mapCopyObj, x, y-1) && isWall(mapCopyObj, x+1, y) ) ||
                        (isWall(mapCopyObj, x+1, y) && isWall(mapCopyObj, x, y+1) ) ||
                        (isWall(mapCopyObj, x, y+1) && isWall(mapCopyObj, x-1, y) ) ||
                        (isWall(mapCopyObj, x-1, y) && isWall(mapCopyObj, x, y-1) )  )
                {
                    mapCopyObj[x][y] = 'x';
                }
            }
            else if(mapCopyObj[x][y] == QChar(' ') &&
                    rand()%100 < GAME::OUTSIDE_DECORATION_PCT)
            {
                //mapObjCopy[x][y] = random.choice(list(OUTSIDEDECOMAPPING.keys()))
                int size = OUTSIDEDECOMAPPING.size();
                int randomIdx = rand() % size;
                QChar c = OUTSIDEDECOMAPPING.keys().at(randomIdx).toLatin1()[0];
                mapCopyObj[x][y] = c;
            }

        }
    }
    return mapCopyObj;
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
