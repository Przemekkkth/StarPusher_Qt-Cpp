#include "pixmapmanager.h"

PixmapManager* PixmapManager::ptr = nullptr;

PixmapManager *PixmapManager::Instance()
{
    if(!ptr)
    {
        ptr = new PixmapManager();
    }
    return ptr;
}

QPixmap& PixmapManager::getPixmap(TextureID id)
{
    return m_textures.get(id);
}

PixmapManager::PixmapManager()
{
    m_textures.load(TextureID::UncoveredGoal, ":/res/sprite/redselector.png");
    m_textures.load(TextureID::CoveredGoal, ":/res/sprite/selector.png");
    m_textures.load(TextureID::Star, ":/res/sprite/star.png");
    m_textures.load(TextureID::Corner, ":/res/sprite/wall_block_tall.png");
    m_textures.load(TextureID::Wall, ":/res/sprite/wood_block_tall.png");
    m_textures.load(TextureID::InsideFloor, ":/res/sprite/plain_block.png");
    m_textures.load(TextureID::OutsideFloor, ":/res/sprite/grass_block.png");
    m_textures.load(TextureID::Title, ":/res/sprite/star_title.png");
    m_textures.load(TextureID::Solved, ":/res/sprite/star_solved.png");
    m_textures.load(TextureID::Princess, ":/res/sprite/princess.png");
    m_textures.load(TextureID::Boy, ":/res/sprite/boy.png");
    m_textures.load(TextureID::Catgirl, ":/res/sprite/catgirl.png");
    m_textures.load(TextureID::Horngirl, ":/res/sprite/horngirl.png");
    m_textures.load(TextureID::Pinkgirl, ":/res/sprite/pinkgirl.png");
    m_textures.load(TextureID::Rock, ":/res/sprite/rock.png");
    m_textures.load(TextureID::ShortTree, ":/res/sprite/tree_short.png");
    m_textures.load(TextureID::TallTree, ":/res/sprite/tree_tall.png");
    m_textures.load(TextureID::UglyTree, ":/res/sprite/tree_ugly.png");
}
