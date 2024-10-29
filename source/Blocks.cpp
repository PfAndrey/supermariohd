#include <unordered_set>

#include <SFML/Graphics.hpp>

#include "Blocks.hpp"
#include "Enemies.hpp"
#include "Pickups.hpp"
#include "SuperMarioGame.hpp"


namespace
{

const Vector BLOCK_SIZE(32, 32);
static std::unordered_set<int> NIGHT_FILTER_EXCEPT = { 82,83,50,58,62,64 };

} // anonymous namespace

SpriteSheet AbstractBlock::s_questionBlockSprite = SpriteSheet();
SpriteSheet AbstractBlock::s_waterSprite = SpriteSheet();
SpriteSheet AbstractBlock::s_lavaSprite = SpriteSheet();
SpriteSheet AbstractBlock::s_staticTiles = SpriteSheet();

AbstractBlock::AbstractBlock(TileCode id)
    : m_id(id)
    , m_invisible(0)
    , m_colliable(0) {

    if (s_staticTiles.empty()) {
        s_staticTiles.load(*MARIO_GAME.textureManager().get("Tiles"), Vector::ZERO, BLOCK_SIZE, 8, 12);
    }

    auto idNum = static_cast<int>(id);
    m_colliable = (idNum <= 40) && (idNum != 14) && (idNum != 22);
}

void AbstractBlock::init() {
    auto& atlas = *MARIO_GAME.textureManager().get("AnimTiles");
    AbstractBlock::s_questionBlockSprite.load(atlas, { 0,0 }, BLOCK_SIZE, 4, 1, 0.005f);
    AbstractBlock::s_waterSprite.load(atlas, { 0,32 }, BLOCK_SIZE, 4, 1, 0.005f);
    AbstractBlock::s_lavaSprite.load(atlas, { 0,64 }, BLOCK_SIZE, 4, 1, 0.005f);
}

void AbstractBlock::killCharactersAbove(Character* attacker) {
    Rect block_rect(m_position - Vector(0,16), BLOCK_SIZE);
 
    const auto& objects = m_blocks->getParent()->getChilds();

    for (auto obj : objects) {
        if (obj->getBounds().isIntersect(block_rect)) {
            if (obj->isTypeOf<Enemy>()) {
                auto enemy = obj->castTo<Enemy>();
                if (enemy->isAlive()) {
                    enemy->takeDamage(DamageType::HIT_FROM_BELOW, attacker);
                }
            } else if (obj->isTypeOf<Mushroom>()) {
                obj->castTo<Mushroom>()->kick();
            } else  if (obj->isTypeOf<Coin>()) {
                obj->castTo<Coin>()->kick();
            }
        }
    };
}

AbstractBlock::~AbstractBlock() {
};

void AbstractBlock::update(int delta_time) {
}

void AbstractBlock::setPosition(const Vector& pos) {
    m_position = pos;
}

bool AbstractBlock::isColliable() const {
    return m_colliable;
}

void AbstractBlock::setParent(Blocks* blocks) {
    m_blocks = blocks;
}

TileCode AbstractBlock::code() const {
    return m_id;
}

bool AbstractBlock::isInvisible() const {
    return m_invisible;
}

void AbstractBlock::setInvisible(bool value) {
    m_invisible = value;
    m_colliable = !value;
}

bool isInvis(TileCode id) {
    return (id == TileCode::INVIZ_UP) ||
           (id == TileCode::INVIZ_COIN) ||
           (id == TileCode::INVIZ_LADDER);
}

void AbstractBlock::drawTileSprite(TileCode id, const Vector& pos, sf::RenderWindow* wnd) {
    SpriteSheet* spriteSheet = nullptr;

    switch (id)
    {
    case TileCode::INVIZ_UP:       // fall through
    case TileCode::INVIZ_COIN:     // fall through
    case TileCode::INVIZ_LADDER:
        // skip drawing
        return;
    case TileCode::BRICK:          // fall through
    case TileCode::COIN_BOX:       // fall through
    case TileCode::BRICK_LADDER:   // fall through
    case TileCode::BRICK_LIVE_UP:  // fall through
    case TileCode::BRICK_MUSHROOM: // fall through
    case TileCode::BRICK_STAR:
        id = TileCode::BRICK;
        break;
    case TileCode::QUESTION_ONE_COIN: // fall through
    case TileCode::QUESTION_MUSHROOM:
        spriteSheet = &s_questionBlockSprite;
        break;
    case TileCode::WATER:
        spriteSheet = &s_waterSprite;
        break;
    case TileCode::LAVA:
        spriteSheet = &s_lavaSprite;
        break;
    default:
        break;
    }

    if (!spriteSheet)
    {
        spriteSheet = &s_staticTiles;
        spriteSheet->setSpriteIndex(static_cast<int>(id) - 1);
    }

    spriteSheet->setPosition(pos);
    spriteSheet->draw(wnd);
}
//---------------------------------------------------------------------------
//! StaticBlock
//---------------------------------------------------------------------------
StaticBlock::StaticBlock(TileCode id)
    : AbstractBlock(id) {
}

void StaticBlock::draw(sf::RenderWindow* render_window) {
    drawTileSprite(code(), m_position, render_window);
};

void StaticBlock::hit(Mario* mario) {
    MARIO_GAME.playSound("bump");
}
//---------------------------------------------------------------------------
//! BricksBlock
//---------------------------------------------------------------------------
BrickBlock::BrickBlock()
    : AbstractBlock(TileCode::BRICK) {
}

void BrickBlock::draw(sf::RenderWindow* render_window) {
    drawTileSprite(code(), m_position + Vector::UP * m_kickedDiff, render_window);
};

void BrickBlock::update(int delta_time) {
    if (m_kickedDir) {
        m_kickedDiff -= m_kickedDir * delta_time / 10;

        if ((m_kickedDir == -1) && (m_kickedDiff > 15)) {  // rising up until peak
            m_kickedDir = 1;
        } else if ((m_kickedDir == 1) && (m_kickedDiff < 0)) { // falling down until init pos
            m_kickedDiff = 0;
            m_kickedDir = 0;
        }
    }
}

void BrickBlock::hit(Mario* mario)
{
    float x = m_position.x;
    float y = m_position.y;

    static const std::pair<Vector, Vector> bricks[] =
    {
        // relative position, speed vector
        { Vector(0, 0),   Vector(-0.05f, -0.2f) },
        { Vector(16, 0),  Vector(0.05f, -0.2f)  },
        { Vector(0, 16),  Vector(-0.05f, -0.1f) },
        { Vector(16, 16), Vector(0.05f, -0.1f)  }
    };

    if (!mario->isSmall()) { // crash box
        for (auto& brick : bricks) {
            MARIO_GAME.spawnObject<OneBrick>(m_position + brick.first, brick.second);
        }

        m_blocks->clearBlock(static_cast<int>(x / BLOCK_SIZE.x), static_cast<int>(y / BLOCK_SIZE.y));
        MARIO_GAME.addScore(50);
        MARIO_GAME.playSound("breakblock");
    } else if (!m_kickedDir) { // just kick box
        m_kickedDiff = 1;
        m_kickedDir = -1;
        MARIO_GAME.playSound("bump");
    }

    killCharactersAbove(mario);
}
//---------------------------------------------------------------------------
//! CoinBox
//---------------------------------------------------------------------------
CoinBoxBlock::CoinBoxBlock(TileCode id)
    : AbstractBlock(id) {
}

void CoinBoxBlock::draw(sf::RenderWindow* render_window) {
    auto id = m_coinLeft ? code()
                         : TileCode::KICKED_BOX;

    drawTileSprite(id, m_position + Vector::UP * m_kickedValue, render_window);
};

void CoinBoxBlock::update(int delta_time) {
    if (!m_kickedValue) {
        return;
    }

    if (m_kickedDir == 0) {
        m_kickedValue += delta_time / 10;
        if (m_kickedValue > 15) {
            m_kickedDir = 1;
        }
    } else if (m_kickedDir == 1) {
        m_kickedValue -= delta_time / 10;
        if (m_kickedValue < 0) {
            m_kickedValue = 0;
        }
    }
}

void CoinBoxBlock::hit(Mario* mario)
{
    if ((m_kickedValue == 0) && m_coinLeft) {
        m_kickedValue = 1;
        m_kickedDir = 0;
        --m_coinLeft;
        Vector pos = m_position + Vector::UP * BLOCK_SIZE.y;

        MARIO_GAME.spawnObject<TwistedCoin>(pos);
        MARIO_GAME.addScore(100, pos);
        MARIO_GAME.addCoin();
        MARIO_GAME.playSound("bump");
        killCharactersAbove(mario);
    }

    if (isInvisible()) {
        setInvisible(false);
    }
}
//---------------------------------------------------------------------------
//! QuestionBlock
//---------------------------------------------------------------------------
QuestionBlock::QuestionBlock(TileCode id, PrizeFabricFunct prizeFabricFunct)
    : AbstractBlock(id)
    , m_prizeFabricFunct(prizeFabricFunct) {
    setInvisible(isInvis(id));
}

void QuestionBlock::draw(sf::RenderWindow* render_window) {
    if (!isInvisible()) {
        auto id = !m_kicked ? code()
                            : TileCode::KICKED_BOX;

        drawTileSprite(id, m_position + Vector::UP * m_kickedValue, render_window);
    }
}

void QuestionBlock::hit(Mario* mario) {
    if (!m_kicked) {
        m_kickedValue = 1;
        m_kickedDir = 0;
        m_kicked = true;
        Vector pos = m_position + Vector::UP * BLOCK_SIZE.y;
        GameObject* object = m_prizeFabricFunct(pos);
        MARIO_GAME.currentScene()->addChild(object);
        killCharactersAbove(mario);
    }

    MARIO_GAME.playSound("bump");
    if (isInvisible()) {
        setInvisible(false);
    }
}

void QuestionBlock::update(int delta_time) {
    if (!m_kickedValue) {
        return;
    }

    if (m_kickedDir == 0) {
        m_kickedValue += delta_time / 10;
        if (m_kickedValue > 15) {
            m_kickedDir = 1;
        }
    } else if (m_kickedDir == 1) {
        m_kickedValue -= delta_time / 10;
        if (m_kickedValue < 0) {
            m_kickedValue = 0;
        }
    }
}
//---------------------------------------------------------------------------
//! Background
//---------------------------------------------------------------------------
Background::Background() {
    m_background.setTextureRect({0, 0, 1280, 720});
}
//---------------------------------------------------------------------------
void Background::draw(sf::RenderWindow* render_window) {
    render_window->draw(m_background);
}
//---------------------------------------------------------------------------
void Background::update(int delta_time) {
    Rect cameraRect = getParent()->castTo<MarioGameScene>()->cameraRect();
    setBounds(cameraRect);
    m_background.setPosition(cameraRect.leftTop());
}
//---------------------------------------------------------------------------
void Background::onStarted() {
    const std::string picture_name = getProperty("Picture").asString();
    const bool nightViewOn = getProperty("NightViewFilter").isValid() && getProperty("NightViewFilter").asBool();

    auto texture = MARIO_GAME.textureManager().get(picture_name);

    m_background.setTexture(*texture);
    m_background.setTextureRect({ 0, 0, 1280, 720 });
    setSize({10, 10});
    getParent()->findChildObjectByType<Blocks>()->enableNightViewFilter(nightViewOn);
    moveToBack();
}
//---------------------------------------------------------------------------
//! Blocks
//---------------------------------------------------------------------------
Blocks::Blocks(int cols, int rows, int tile_width, int tile_height) {
    setName("Blocks");
    m_tile_map = new TileMap<AbstractBlock*>(cols, rows, tile_width, tile_height);
    m_tile_map->clear(nullptr);

    AbstractBlock::init();

    static const sf::String frag_shader =
        "#version 120\n"\
        "uniform sampler2D texture;\n"\
        "void main()"\
        "{"\
        "   vec4 color = texture2D(texture, gl_TexCoord[0].xy);"\
        "   float middle = (color.r + color.g)/2.f;"\
        "   gl_FragColor = vec4(color.g,middle,middle,color.a);"\
        "}";
 
    m_nightViewFilterShader.loadFromMemory(frag_shader, sf::Shader::Fragment);
    m_nightViewFilterShader.setUniform("texture", sf::Shader::CurrentTexture);
}

void Blocks::loadFromArray(const std::vector<char>& data, std::function<AbstractBlock*(char)> fabric) {
   // m_tile_map->loadFromArray<char>(fabric, data);

    if (data.size() != m_tile_map->cols() * m_tile_map->rows()) {
        throw std::runtime_error("Blocks - invalid data size");
    }

    int block_cols = m_tile_map->cols();

    for (int i = 0; i < data.size(); ++i) {
        char id = data[i];

        if (!id) {
            continue;
        }

        auto block = fabric(id);
        int x = i % block_cols;
        int y = i / block_cols;

        m_tile_map->setTile(x, y, block);

        if (block) {
            block->setPosition(Vector(x, y) * BLOCK_SIZE.x);
            block->setParent(this);
        }
    }
 

    setPosition({0, 0});
    setSize(m_tile_map->getRenderBounds().size());
}

void Blocks::enableNightViewFilter(bool enable) {
    m_nightViewFilter = enable;
}

void Blocks::draw(sf::RenderWindow* render_window) {
    if (m_nightViewFilter) {
        sf::Shader::bind(&m_nightViewFilterShader);
    }

    Rect cameraRect = getParent()->castTo<MarioGameScene>()->cameraRect();
    const float block_size = blockSize().x;
    Vector center = render_window->getView().getCenter();
    Vector size = render_window->getView().getSize();
 
    m_viewRect = Rect(toBlockCoordinates((center - size / 2)), toBlockCoordinates(size)).getIntersection(getRenderBounds());
    m_viewRect.setWidth(m_viewRect.width() + 2);

    forEachVisibleBlock([=](AbstractBlock* block, int, int) {
            int idNum = static_cast<int>(block->code());
            bool skipFilter = m_nightViewFilter && NIGHT_FILTER_EXCEPT.count(idNum);

            if (skipFilter)
                sf::Shader::bind(nullptr);

            block->draw(render_window);

            if (skipFilter) {
                sf::Shader::bind(&m_nightViewFilterShader);
            }
        });

    GameObject::draw(render_window);

    if (m_nightViewFilter) {
        sf::Shader::bind(nullptr);
    }
}

void Blocks::update(int delta_time) {
    if (!m_removeLaterList.empty()) {
        for (auto object : m_removeLaterList) {
            delete object;
        }
        m_removeLaterList.clear();
    }

    GameObject::update(delta_time);

    AbstractBlock::s_questionBlockSprite.update(delta_time);
    AbstractBlock::s_waterSprite.update(delta_time);
    AbstractBlock::s_lavaSprite.update(delta_time);

    forEachVisibleBlock([=](AbstractBlock* block, int, int) {
            block->update(delta_time);
        });
}

Rect Blocks::getBlockBounds(const Vector& block) const {
    return Rect(Vector(block.x * BLOCK_SIZE.x, block.x * BLOCK_SIZE.y), BLOCK_SIZE);
}

const Vector& Blocks::blockSize() const {
    return BLOCK_SIZE;
}

AbstractBlock* Blocks::getBlock(int x, int y) {
    return m_tile_map->getTile(x, y);
}

void Blocks::clearBlock(int x, int y) {
    m_removeLaterList.push_back(m_tile_map->getTile(x, y));
    m_tile_map->setTile(x, y, nullptr);
}

void Blocks::hitBlock(int x, int y, Mario* mario) {
    AbstractBlock* block = getBlock(x, y);
    if (!block) {
        return;
    }
 
    block->hit(mario);
}

int Blocks::rows() const {
    return m_tile_map->rows();
}

int Blocks::cols() const {
    return m_tile_map->cols();
}

Rect Blocks::getRenderBounds() const {
    return m_tile_map->getRenderBounds();
}

bool Blocks::isCollidableBlock(const Vector& blockPos) const {
    AbstractBlock* block = m_tile_map->getTile(blockPos.x, blockPos.y);
    return block && block->isColliable();
}

bool Blocks::isInvizibleBlock(const Vector& blockPos) const {
    AbstractBlock* block = m_tile_map->getTile(blockPos.x, blockPos.y);
    return block && block->isInvisible();
}

Vector Blocks::toBlockCoordinates(const Vector& pixel) const {
    return m_tile_map->getTileFromPointCoordinates(pixel);
}

Vector Blocks::toPixelCoordinates(const Vector& block) const {
    return m_tile_map->getPointCoordinatesFromTile(block);
}

bool Blocks::isBlockInBounds(const Vector& block) const {
    return m_tile_map->isTileInBounds(block);
}

Vector Blocks::collsionResponse(const Rect& body_rect, const Vector& body_speed, float delta_time, ECollisionTag& collision_tag) {
    Vector own_size = body_rect.size();
    Vector new_pos = body_rect.leftTop();
    const float tile_size = blockSize().x;

    Rect tilesRect = Rect(body_rect.leftTop(), own_size)
        .moved(-Vector(body_speed.x * delta_time, 0.f))
        .scaled(1 / tile_size);

    // Y axis
    for (int x = tilesRect.left(); x < tilesRect.right(); ++x) {
        for (int y = tilesRect.top(); y < tilesRect.bottom(); ++y) {
            Vector block(x, y);
            if (isBlockInBounds(block) && (isCollidableBlock(block) || (isInvizibleBlock(block) && (body_speed.y < 0)))) {
                if ((body_speed.y == 0) && (body_rect.bottom() > (y + 1) * tile_size)) { // fix for "body in the restricted area" collision bug
                    collision_tag |= ECollisionTag::Y_AXIS;
                    return new_pos + Vector::RIGHT * 2; // push avay body onto right side
                } if (body_speed.y >= 0) {
                    new_pos.y = y * tile_size - own_size.y;
                    collision_tag |= ECollisionTag::FLOOR;
                } else if (body_speed.y < 0) {
                    new_pos.y = y * tile_size + tile_size;
                    collision_tag |= ECollisionTag::CELL;
                }
                break;
            }
        }
    }

    tilesRect = Rect(new_pos, own_size).scaled(1 / tile_size);

    // X axis
    for (int x = tilesRect.left(); x < tilesRect.right(); ++x) {
        for (int y = tilesRect.top(); y < tilesRect.bottom(); ++y) {
            Vector block(x, y);
            if (isBlockInBounds(block) && isCollidableBlock(block)) {
                if (body_speed.x > 0) {
                    new_pos.x = x * tile_size - own_size.x;
                    collision_tag |= ECollisionTag::LEFT;
                } else if (body_speed.x < 0) {
                    new_pos.x = x * tile_size + tile_size;
                    collision_tag |= ECollisionTag::RIGHT;
                }
                break;
            }
        }
    }

    return new_pos;
}

void Blocks::forEachVisibleBlock(const std::function<void(AbstractBlock*, int, int)>& func) {
    for (int x = m_viewRect.left(); x < m_viewRect.right(); ++x) {
        for (int y = m_viewRect.top(); y < m_viewRect.bottom(); ++y) {
            if (!m_tile_map->isTileInBounds(Vector(x, y))) {
                continue;
            }

            AbstractBlock* block = m_tile_map->getTile(x, y);

            if (block) {
                func(block, x, y);
            }
        }
    }
}

Blocks::~Blocks() {
    for (int x = 0; x < m_tile_map->cols(); ++x) {
        for (int y = 0; y < m_tile_map->rows(); ++y) {
            delete m_tile_map->getTile(x, y);
        }
    }

    delete m_tile_map;
}

std::vector<Vector> Blocks::getBridgeBlocks() {
    std::vector<Vector> bridge_cells;

    for (int x = 0; x < m_tile_map->cols(); ++x) {
        for (int y = 0; y < m_tile_map->rows(); ++y) {
            auto* tile = m_tile_map->getTile(x, y);

            if (tile && (tile->code() == TileCode::BRIDGE) ||
                        (tile->code() == TileCode::CHAIN)) {
                   bridge_cells.emplace_back(x, y);
                }
        }
    }

    return bridge_cells;
}
//---------------------------------------------------------------------------
//! OneBrick
//---------------------------------------------------------------------------
OneBrick::OneBrick(const Vector& pos, const Vector& speed_vector) {
    setName("OneBrick");
    setPosition(pos);
    m_speed = speed_vector;
    m_sprite_sheet.load(*MARIO_GAME.textureManager().get("Items"), { { 96,0,16,16 }, { 96,16,16,-16 } });

    m_sprite_sheet.setAnimType(AnimType::FORWARD_CYCLE);
    m_sprite_sheet.setSpeed(0.005f);
}

void OneBrick::draw(sf::RenderWindow* render_window) {
    m_sprite_sheet.setPosition(getPosition());
    m_sprite_sheet.draw(render_window);
}

void OneBrick::update(int delta_time)  {
    m_sprite_sheet.update(delta_time);
    m_speed.y += delta_time * 0.0005f; // gravity
    move(m_speed*delta_time);
    m_timer += delta_time;
    if (m_timer > 3000) {
       removeLater();
    }
}
//---------------------------------------------------------------------------
//! TwistedCoin
//---------------------------------------------------------------------------
TwistedCoin::TwistedCoin(const Vector& pos) {
    auto& texture = *MARIO_GAME.textureManager().get("Items");
    m_animator.create("twist", texture, Vector(0, 84), Vector(32, 32), 4, 1, 0.01f);
    m_animator.create("shine", texture, Vector(0, 116), Vector(40, 32), 5, 1, 0.01f, AnimType::FORWARD);
    m_animator.get("shine")->setOrigin(Vector(4, 0));

    setPosition(pos);
    m_speed = Vector::UP * 0.05f;
    m_speed.y = -0.20f;
    MARIO_GAME.addScore(100, pos);
    MARIO_GAME.addCoin();
}

void TwistedCoin::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

void TwistedCoin::update(int delta_time) {
    if (m_timer == 0) {
        MARIO_GAME.playSound("coin");
    } else if (m_timer < 700) {
        m_speed.y += delta_time * 0.0005f; // gravity
        move(m_speed * delta_time);
    } else if (m_timer < 1200) {
        m_animator.play("shine");
    } else {
        removeLater();
    }

    m_timer += delta_time;
    m_animator.update(delta_time);
}

template <>
PrizeFabricFunct prize<Mushroom>() {
    return [](const Vector& pos) -> GameObject* {
            if (MARIO_GAME.getPlayer()->isSmall()) {
                return new Mushroom(pos);
            } else {
                return new FireFlower(pos);
            }
        };
}
