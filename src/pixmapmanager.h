#ifndef PIXMAPMANAGER_H
#define PIXMAPMANAGER_H
#include "resource_holder.h"
#include <QPixmap>

class PixmapManager
{
public:
    enum class TextureID{
        UncoveredGoal, CoveredGoal,
        Star, Corner, Wall, InsideFloor,
        OutsideFloor, Title, Solved,
        Princess, Boy, Catgirl, Horngirl,
        Pinkgirl, Rock, ShortTree, TallTree,
        UglyTree
    };
    static PixmapManager* Instance();
    QPixmap& getPixmap(TextureID id);
private:
    PixmapManager();
    PixmapManager(PixmapManager& other) = delete;
    void operator=(const PixmapManager &) = delete;

    ResourceHolder<QPixmap, TextureID> m_textures;
    static PixmapManager* ptr;
};

#endif // PIXMAPMANAGER_H
