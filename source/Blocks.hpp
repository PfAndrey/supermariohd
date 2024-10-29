#ifndef BLOCKS_HPP
#define BLOCKS_HPP

#include "GameEngine.hpp"
#include "TileMap.hpp"
#include "Items.hpp"
#include "SuperMarioGame.hpp"

class Mario;

/// @brief Blocks tiled codes from tiled map.
enum class TileCode : uint8_t {
    EMPTY               = 0,  // !< Emtpty block
    BRICK               = 2,  // !< Brick block
    COIN_BOX            = 3,  // !< Coin box block
    KICKED_BOX          = 9,  // !< Kicked box block
    QUESTION_ONE_COIN   = 11, // !< Question block with coin
    QUESTION_MUSHROOM   = 12, // !< Question block with mushroom
    BRICK_STAR          = 13,
    LADDER              = 17,
    BRIDGE              = 18,
    CHAIN               = 81,
    INVIZ_UP            = 20,
    INVIZ_COIN          = 19,
    BRICK_LIVE_UP       = 21,
    BRICK_LADDER        = 26,
    INVIZ_LADDER        = 27,
    BRICK_MUSHROOM      = 28,

    WATER               = 50,
    LAVA                = 81
};

/// @brief Base class for all blocks.
class AbstractBlock {
public:
    AbstractBlock(TileCode id);
    virtual ~AbstractBlock();
    virtual void draw(sf::RenderWindow* render_window) = 0;
    virtual void update(int delta_time);
    virtual void hit(Mario* mario) = 0;
    void setPosition(const Vector& pos);
    bool isColliable() const;
    bool isInvisible() const;
    void setParent(Blocks* blocks);
    TileCode code() const;

    static void init();
protected:

    void setInvisible(bool value);
    void killCharactersAbove(Character* attacker);
    void drawTileSprite(TileCode id, const Vector& pos, sf::RenderWindow* wnd);
    class Blocks* m_blocks = nullptr;
    Vector m_position;

private:

    TileCode m_id = TileCode::EMPTY;
    // animated tiles
    static SpriteSheet s_questionBlockSprite;
    static SpriteSheet s_waterSprite;
    static SpriteSheet s_lavaSprite;
    // static tiles
    static SpriteSheet s_staticTiles;
    bool m_invisible : 1;
    bool m_colliable : 1;
    friend class Blocks;
};


class StaticBlock : public AbstractBlock {
public:
    StaticBlock(TileCode id);
    void draw(sf::RenderWindow* render_window) override;
    void hit(Mario* mario) override;
};

class BrickBlock : public AbstractBlock {
public:
    BrickBlock();
    void draw(sf::RenderWindow* render_window) override;
    void hit(Mario* mario) override;
    void update(int delta_time) override;

private:
    float m_kickedDiff = 0;
    int m_kickedDir = 0;  //!< -1 - up, 1 - down, 0 - no
};

class CoinBoxBlock : public AbstractBlock {
public:
    CoinBoxBlock(TileCode id);
    void draw(sf::RenderWindow* render_window) override;
    void hit(Mario* mario) override;
    void update(int delta_time) override;

private:
    int m_coinLeft = 5;
    float m_kickedValue = 0;
    int m_kickedDir = 0;
};

/// @brief Prize Fabric Function
using PrizeFabricFunct = std::function<GameObject* (const Vector& pos)>;


class QuestionBlock : public AbstractBlock {
public:
    QuestionBlock(TileCode id, PrizeFabricFunct prizeFabricFunct);
    void draw(sf::RenderWindow* render_window) override;
    void hit(Mario* mario) override;
    void update(int delta_time) override;

protected:
    PrizeFabricFunct m_prizeFabricFunct;
    bool m_kicked = false;
    float m_kickedValue = 0;
    int m_kickedDir = 0;
};


class Blocks : public GameObject {
    DECLARE_TYPE_INFO(GameObject)

public:
    Blocks(int cols, int rows, int tile_width, int tile_height);
    ~Blocks();
    const Vector& blockSize() const;
    AbstractBlock* getBlock(int x, int y);
    int rows() const;
    int cols() const;
    Rect getRenderBounds() const;
    Vector toBlockCoordinates(const Vector& pixel) const;
    Vector toPixelCoordinates(const Vector& block) const;
    Rect getBlockBounds(const Vector& block) const;
    bool isBlockInBounds(const Vector& block) const;
    std::vector<Vector> getBridgeBlocks();
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    void clearBlock(int x, int y);
    void hitBlock(int x, int y, Mario* mario);
    void enableNightViewFilter(bool enable);
    void loadFromArray(const std::vector<char>& data, std::function<AbstractBlock* (char)> fabric);
    bool isCollidableBlock(const Vector& block) const;
    bool isInvizibleBlock(const Vector& block) const;
    Vector collsionResponse(const Rect& body_rect, const Vector& body_speed, float delta_time, ECollisionTag& collision_tag);

private:

    void forEachVisibleBlock(const std::function<void(AbstractBlock*, int, int)>& func);
    Rect m_viewRect;
    TileMap<AbstractBlock*>* m_tile_map;
    //sf::RectangleShape m_shape;
    sf::Shader m_nightViewFilterShader;
    bool m_nightViewFilter = false;
    std::vector<AbstractBlock*> m_removeLaterList;
};

class Background : public GameObject {
    DECLARE_TYPE_INFO(GameObject)

public:
    Background();
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;

private:
    void onStarted() override;
    sf::Sprite m_background;
};

class OneBrick : public GameObject {
 public:
    OneBrick(const Vector& pos, const Vector& speed_vector);
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;

 private:
    Vector m_speed;
    SpriteSheet m_sprite_sheet;
    int m_timer = 0;
};


class TwistedCoin : public GameObject {
public:
    TwistedCoin(const Vector& pos);
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;

private:
    Vector m_speed;
    Animator m_animator;
    int m_timer = 0;
};

//---------------------------------------------------------------------------
// Inline Functions
//---------------------------------------------------------------------------

template <typename T>
PrizeFabricFunct prize()
{
    return [](const Vector& pos) { return new T(pos); };
}

template <>
inline PrizeFabricFunct prize<Ladder>()
{
    return [](const Vector& pos)
    {
        GameObject* ladder = new Ladder();
        ladder->setPosition(pos + Vector(10, 0));
        return ladder;
    };
}

class Mushroom;

template <>
PrizeFabricFunct prize<Mushroom>();

#endif // BLOCKS_HPP
