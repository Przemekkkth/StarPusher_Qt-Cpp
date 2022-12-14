#include "gamescene.h"
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QGraphicsSimpleTextItem>
#include <QPainter>
#include <QDir>
#include "utils.h"
#include "pixmapmanager.h"
#include "game_state.h"

bool KeyStatus::s_keyPressed = false;

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
      m_currentImageIndex(0),
      m_mapNeedsRedraw(false),
      m_levelIsCompleted(false),
      m_cameraOffsetX(0),
      m_cameraOffsetY(0)
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
    readLevelsFile(":/res/lvl/starPusherLevels.txt");
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
        if(m_mapNeedsRedraw)
        {
            clear();
            drawMap();
            if(m_levelIsCompleted)
            {
                QRect screenRect = QRect(SCREEN::HALF_WIDTH/2-m_cameraOffsetX,SCREEN::HALF_WIDTH/4-m_cameraOffsetY,SCREEN::HALF_WIDTH/2, SCREEN::HALF_WIDTH/2);
                drawTilemap(screenRect, PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::Solved));
            }
            drawCurentLevelStatus();
            drawStepCounter();
            m_mapNeedsRedraw = false;
        }
        if(m_levelIsCompleted)
        {
            if(KeyStatus::s_keyPressed)
            {
                nextLevel();
            }
        }
        resetStatus();
    }
}

void GameScene::handlePlayerInput()
{
    QString playerMove = GAME::NONE;
    int mapWidth = m_mapObj.length() * GAME::TILEWIDTH;
    int mapHeight = (m_mapObj[0].length() - 1) * GAME::TILEFLOORHEIGHT + GAME::TILEHEIGHT;
    bool cameraUp = false, cameraDown = false, cameraLeft = false, cameraRight = false;
    int MAX_CAM_X_PAN = std::abs(SCREEN::HALF_HEIGHT - int(mapHeight)) + GAME::TILEWIDTH;
    int MAX_CAM_Y_PAN = std::abs(SCREEN::HALF_WIDTH- int(mapWidth)) + GAME::TILEHEIGHT;

    if(m_keys[KEYBOARD::KEY_DOWN]->m_released)
    {
        playerMove = GAME::DOWN;
    }
    else if(m_keys[KEYBOARD::KEY_RIGHT]->m_released)
    {
        playerMove = GAME::RIGHT;
    }
    else if(m_keys[KEYBOARD::KEY_LEFT]->m_released)
    {
        playerMove = GAME::LEFT;
    }
    else if(m_keys[KEYBOARD::KEY_UP]->m_released)
    {
        playerMove = GAME::UP;
    }
    else if(m_keys[KEYBOARD::KEY_A]->m_held)
    {
        //camera left
        cameraLeft = true;
    }
    else if(m_keys[KEYBOARD::KEY_D]->m_held)
    {
        //camera right
        cameraRight = true;
    }
    else if(m_keys[KEYBOARD::KEY_S]->m_held)
    {
        //camera down
        cameraDown = true;
    }
    else if(m_keys[KEYBOARD::KEY_W]->m_held)
    {
        //camera up
        cameraUp = true;
    }
    else if(m_keys[KEYBOARD::KEY_N]->m_released)
    {
        //next
        nextLevel();
    }
    else if(m_keys[KEYBOARD::KEY_B]->m_released)
    {
        //next
        previousLevel();
    }
    else if(m_keys[KEYBOARD::KEY_R]->m_released)
    {
        //reset
        runLevel();
    }
    else if(m_keys[KEYBOARD::KEY_P]->m_released)
    {
        //backspace 'reset'
        m_currentImageIndex += 1;
        if(m_currentImageIndex >= PLAYERIMAGES.length())
        {
            m_currentImageIndex = 0;
        }
        m_mapNeedsRedraw = true;
    }
    else if(m_keys[KEYBOARD::KEY_Z]->m_released)
    {
        //render scene to image
        //renderGameScene();
    }

    if(playerMove != GAME::NONE && !m_levelIsCompleted)
    {
        bool moved = makeMove(playerMove);
        if(moved)
        {
            m_gameStateObj.stepCounter += 1;
            m_mapNeedsRedraw = true;
        }
        if(isLevelFinished())
        {
            m_levelIsCompleted = true;
        }
    }

    if( cameraUp && m_cameraOffsetY < MAX_CAM_X_PAN)
    {
        m_cameraOffsetY += GAME::CAM_MOVE_SPEED;
        m_mapNeedsRedraw = true;
    }
    else if(cameraDown && m_cameraOffsetY > -MAX_CAM_X_PAN)
    {
        m_cameraOffsetY -= GAME::CAM_MOVE_SPEED;
        m_mapNeedsRedraw = true;
    }
    if(cameraLeft && m_cameraOffsetX < MAX_CAM_Y_PAN)
    {
        m_cameraOffsetX += GAME::CAM_MOVE_SPEED;
        m_mapNeedsRedraw = true;
    }
    else if(cameraRight && m_cameraOffsetX > -MAX_CAM_Y_PAN)
    {
        m_cameraOffsetX -= GAME::CAM_MOVE_SPEED;
        m_mapNeedsRedraw = true;
    }
    setSceneRect(-SCREEN::HALF_WIDTH/2 - m_cameraOffsetX, -SCREEN::HALF_HEIGHT/2 - m_cameraOffsetY, SCREEN::PHYSICAL_SIZE.width(), SCREEN::PHYSICAL_SIZE.height());
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
            if(line.startsWith(";"))
            {
                continue;
            }
            if(!line.isEmpty())
            {
                dataContent.push_back(line);
            }
            else if (line.isEmpty() && dataContent.length() > 0)
            {
                if(!dataContent.size())
                {
                    continue;
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
                mapObj.resize(maxWidth);

                for(int y = 0; y < dataContent.size(); ++y)
                {
                    for(int x = 0; x < maxWidth; ++x)
                    {
                        if(dataContent[y][x] == QChar('-'))
                        {
                            dataContent[y][x] = QChar(' ');
                        }
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
                ///clear
                dataContent.clear();
                maxWidth = -1;
                mapObj.clear();
            }
        }
    }
    else{
        qDebug() << "file is not open";
    }
    file.close();
}

bool GameScene::isWall(int x, int y)
{
    if(x < 0 || x >= m_mapObj.length() || y < 0 || y >= m_mapObj[x].length())
    {
        return false;
    }
    else if(m_mapObj[x][y] == QChar('#') || m_mapObj[x][y] == QChar('x'))
    {
        return true;
    }
    return false;
}

void GameScene::floodFill(int x, int y, QChar oldChar, QChar newChar)
{
    if(m_mapObj[x][y] == oldChar)
    {
        m_mapObj[x][y] = newChar;
    }

    if(x < m_mapObj.length()-1 && m_mapObj[x+1][y] == oldChar)
    {
        floodFill(x+1, y, oldChar, newChar); // call right
    }
    if(x > 0 && m_mapObj[x-1][y] == oldChar)
    {
        floodFill(x-1, y, oldChar, newChar); // call left
    }
    if(y < m_mapObj[x].length() && m_mapObj[x][y+1] == oldChar)
    {
        floodFill(x, y+1, oldChar, newChar); // call down
    }
    if(y > 0 && m_mapObj[x][y-1] == oldChar)
    {
        floodFill(x, y-1, oldChar, newChar); // call up
    }
}

void GameScene::runLevel()
{
    m_mapObj.clear();


    m_levelObj = m_levels[m_currentLevelIndex];
    m_mapObj = m_levelObj.mapObj;
    m_gameStateObj = m_levelObj.startState;

    decorateMap();
    m_mapNeedsRedraw = true;
    m_cameraOffsetX = m_cameraOffsetY = 0;
    m_levelIsCompleted = false;
}

void GameScene::drawMap()
{
    clear();
    setBackgroundBrush(GAME::BGCOLOR);
    QList<QPoint> goals = m_levelObj.goals;
    for(int x = 0; x < m_mapObj.length(); ++x)
    {
        for(int y = 0; y < m_mapObj[x].length(); ++y)
        {
            QRect spaceRect = QRect(x*GAME::TILEWIDTH+m_cameraOffsetX, y*GAME::TILEFLOORHEIGHT+m_cameraOffsetY,
                                    GAME::TILEWIDTH, GAME::TILEHEIGHT);
            QPixmap pixmap;
            bool isDraw = false;
            if(TILEMAPPING.contains(QString(m_mapObj[x][y])))
            {
                //baseTile = TILEMAPPING[m_mapObj[x][y]]
                pixmap = TILEMAPPING[QString(m_mapObj[x][y])];
                isDraw = true;
            }
            else if(OUTSIDEDECOMAPPING.contains(QString(m_mapObj[x][y])))
            {
                pixmap = TILEMAPPING[" "];
                isDraw = true;
            }
            if(isDraw)
            {
                drawTilemap(spaceRect, pixmap);
            }

            if(OUTSIDEDECOMAPPING.contains(QString(m_mapObj[x][y])))
            {
                pixmap = OUTSIDEDECOMAPPING[QString(m_mapObj[x][y])];
                drawTilemap(spaceRect, pixmap);
            }
            else if(m_gameStateObj.stars.contains(QPoint(x,y)))
            {
                if(goals.contains(QPoint(x,y)))
                {
                    pixmap = PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::CoveredGoal);
                    drawTilemap(spaceRect, pixmap);
                }
                pixmap = PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::Star);
                drawTilemap(spaceRect, pixmap);
            }
            else if(goals.contains(QPoint(x,y)))
            {
                pixmap = PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::UncoveredGoal);
                drawTilemap(spaceRect, pixmap);
            }

            if(m_gameStateObj.player == QPoint(x,y))
            {
                pixmap = PLAYERIMAGES.at(m_currentImageIndex);
                drawTilemap(spaceRect, pixmap);
            }
        }
    }
}

bool GameScene::isBlocked(int x, int y)
{
    if(isWall(x, y))
    {
        return true;
    }
    else if(x < 0 || x >= m_mapObj.length() || y < 0 || y >= m_mapObj[x].length())
    {
        return true;
    }
    else if(m_gameStateObj.stars.contains(QPoint(x,y)))
    {
        return true;
    }
    return false;
}

bool GameScene::makeMove(QString playerMoveTo)
{
    QPoint playerPos = m_gameStateObj.player;
    QList<QPoint> stars = m_gameStateObj.stars;
    int xOffset = 0, yOffset = 0;
    if(playerMoveTo == GAME::UP)
    {
        xOffset = 0;
        yOffset = -1;
    }
    else if(playerMoveTo == GAME::RIGHT)
    {
        xOffset = 1;
        yOffset = 0;
    }
    else if(playerMoveTo == GAME::LEFT)
    {
        xOffset = -1;
        yOffset = 0;
    }
    else if(playerMoveTo == GAME::DOWN)
    {
        xOffset = 0;
        yOffset = 1;
    }

    if( isWall(playerPos.x()+xOffset, playerPos.y()+yOffset))
    {
        return false;
    }
    else
    {
        if(stars.contains(QPoint(playerPos.x() + xOffset, playerPos.y() + yOffset)))
        {
            if(!isBlocked(playerPos.x() + (xOffset*2), playerPos.y() + (yOffset*2)))
            {
                int index = -1;
                for(int i = 0; i < stars.size(); ++i)
                {
                    if(stars.at(i) == QPoint(playerPos.x() + xOffset, playerPos.y() + yOffset))
                    {
                        index = i;
                    }
                }
                if(index != -1)
                {
                   stars[index] = QPoint(stars[index].x()+xOffset, stars[index].y()+yOffset);
                }
            }
            else
            {
                return false;
            }
        }
        m_gameStateObj.stars = stars;
        m_gameStateObj.player = QPoint(playerPos.x()+xOffset, playerPos.y()+yOffset);
        return true;
    }
}

void GameScene::decorateMap()
{
    // Remove the non-wall characters from the map data
    QPoint startPos = m_gameStateObj.player;
    for(int x = 0; x < m_mapObj.length(); ++x)
    {
        for(int y = 0; y < m_mapObj[0].length(); ++y)
        {
            if(     m_mapObj[x][y] == QChar('$') ||
                    m_mapObj[x][y] == QChar('.') ||
                    m_mapObj[x][y] == QChar('@') ||
                    m_mapObj[x][y] == QChar('+') ||
                    m_mapObj[x][y] == QChar('*') )
            {
                m_mapObj[x][y] = QChar(' ');
            }
        }
    }
    // Flood fill to determine inside/outside floor tiles.
    floodFill(startPos.x(), startPos.y(), ' ', 'o');
    //Convert the adjoined walls into corner tiles.
    for(int x = 0; x < m_mapObj.length(); ++x)
    {
        for(int y = 0; y < m_mapObj[0].length(); ++y)
        {
            if(m_mapObj[x][y] == QChar('#') )
            {
                if( (isWall(x, y-1) && isWall(x+1, y) ) ||
                    (isWall(x+1, y) && isWall(x, y+1) ) ||
                    (isWall(x, y+1) && isWall(x-1, y) ) ||
                    (isWall(x-1, y) && isWall(x, y-1) )  )
                {
                    m_mapObj[x][y] = 'x';
                }
            }
            else if(m_mapObj[x][y] == QChar(' ') &&
                    rand()%100 < GAME::OUTSIDE_DECORATION_PCT)
            {
                int size = OUTSIDEDECOMAPPING.size();
                int randomIdx = rand() % size;
                QChar c = OUTSIDEDECOMAPPING.keys().at(randomIdx).toLatin1()[0];
                m_mapObj[x][y] = c;
            }

        }
    }
}

bool GameScene::isLevelFinished()
{
    for(int i = 0; i < m_levelObj.goals.size(); ++i)
    {
        if(!m_gameStateObj.stars.contains(m_levelObj.goals.at(i)))
        {
            return false;
        }
    }
    return true;
}

void GameScene::drawTilemap(QRect drawRect, const QPixmap pixmap)
{
    QGraphicsPixmapItem* pItem = new QGraphicsPixmapItem();
    pItem->setPos(drawRect.x(), drawRect.y());
    pItem->setPixmap(pixmap.scaled(drawRect.width(), drawRect.height()));
    addItem(pItem);
}

void GameScene::nextLevel()
{
    m_currentLevelIndex++;
    if(m_currentLevelIndex >= m_levels.size())
    {
        m_currentLevelIndex = 0;
    }
    runLevel();
}

void GameScene::previousLevel()
{
    m_currentLevelIndex--;
    if(m_currentLevelIndex < 0)
    {
        m_currentLevelIndex = 0;
    }
    runLevel();
}

void GameScene::drawStepCounter()
{
    QString text = "Steps: " + QString::number(m_gameStateObj.stepCounter);
    int mapHeight = (m_mapObj[0].length() - 1) * GAME::TILEFLOORHEIGHT + GAME::TILEHEIGHT;
    QPoint point = QPoint(m_cameraOffsetX,mapHeight+0.05*sceneRect().height());
    QGraphicsSimpleTextItem* tItem = new QGraphicsSimpleTextItem();
    QFont font = tItem->font();
    font.setPixelSize(0.05*sceneRect().height());
    tItem->setFont(font);
    tItem->setTransformOriginPoint(QPoint(0,0));
    tItem->setPen(QColor(GAME::TEXTCOLOR));
    tItem->setBrush(QColor(GAME::TEXTCOLOR));
    tItem->setText(text);
    point.setX(point.x()+m_cameraOffsetX);
    point.setY(point.y()+m_cameraOffsetY);
    tItem->setPos(point);
    addItem(tItem);
}

void GameScene::drawCurentLevelStatus()
{
    QString text = "Level " + QString::number(m_currentLevelIndex) + " of " + QString::number(m_levels.size());
    int mapWidth = m_mapObj.length() * GAME::TILEWIDTH;
    int mapHeight = (m_mapObj[0].length() - 1) * GAME::TILEFLOORHEIGHT + GAME::TILEHEIGHT;
    QPoint point = QPoint(m_cameraOffsetX,mapHeight);
    QGraphicsSimpleTextItem* tItem = new QGraphicsSimpleTextItem();
    QFont font = tItem->font();
    font.setPixelSize(0.05*sceneRect().height());
    tItem->setFont(font);
    tItem->setTransformOriginPoint(QPoint(0,0));
    tItem->setPen(QColor(GAME::TEXTCOLOR));
    tItem->setBrush(QColor(GAME::TEXTCOLOR));
    tItem->setText(text);
    point.setX(point.x()+m_cameraOffsetX);
    point.setY(point.y()+m_cameraOffsetY);
    tItem->setPos(point);
    addItem(tItem);
}

void GameScene::renderGameScene()
{
    static int index = 0;
    QString fileName = QDir::currentPath() + QDir::separator() + "screen" + QString::number(index++) + ".png";
    //QRect rect = sceneRect().toAlignedRect();
    QRect rect = QRect(m_cameraOffsetX, m_cameraOffsetY, SCREEN::PHYSICAL_SIZE.width(), SCREEN::PHYSICAL_SIZE.height());
    QImage image(rect.size(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    render(&painter);
    image.save(fileName);
    qDebug() << "saved " << fileName;
}

void GameScene::keyPressEvent(QKeyEvent *event)
{
    if(KEYBOARD::KeysMapper.contains(event->key()))
    {
        KeyStatus::s_keyPressed = true;
        m_keys[KEYBOARD::KeysMapper[event->key()]]->m_held = true;
    }
    QGraphicsScene::keyPressEvent(event);
}

void GameScene::keyReleaseEvent(QKeyEvent *event)
{
    if(!event->isAutoRepeat())
    {
        if(KEYBOARD::KeysMapper.contains(event->key()))
        {
            KeyStatus::s_keyPressed = false;
            m_keys[KEYBOARD::KeysMapper[event->key()]]->m_held = false;
            m_keys[KEYBOARD::KeysMapper[event->key()]]->m_released = true;
        }
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
