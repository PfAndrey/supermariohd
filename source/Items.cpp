//!
//! \file       Items.cpp
//! \author     Andriy Parfenyuk
//! \date       14.3.2017
//!
//! \brief      Implementation of Items classes.
//!
//! \license    GNU
//!
/////////////////////////////////////////////////////////////////////////////

#include <cmath>

#include <Format.hpp>
#include "Blocks.hpp"
#include "Enemies.hpp"
#include "Items.hpp"
#include "SuperMarioGame.hpp"
#include "Mario.hpp"

namespace {

constexpr int PLATFORM_HEIGHT = 16;

} // anonymous namespace

void Platform::collsionResponse(Mario* mario, ECollisionTag& collision_tag, int delta_time) {
    mario->setPosition(::collisionResponse(mario->getBounds(), mario->getSpeed(), getBounds(),
                                           getSpeedVector(), delta_time, collision_tag));
}

MoveablePlatform::MoveablePlatform()
    : m_sprite(*MARIO_GAME.textureManager().get("Items"),
        {{0,0}, {m_size, 16 }})
 {
    m_platform_type = PlatformType::UNKNOWN;
    m_amplitude = 0;
}

void MoveablePlatform::draw(sf::RenderWindow* render_window)
{
    m_sprite.setPosition(getPosition());
    render_window->draw(m_sprite);
}

void MoveablePlatform::update(int delta_time)
{
    switch (m_platform_type) {
    case PlatformType::ELEVATOR: {
        static const int bottom = getParent()->findChildObjectByType<Blocks>()->getRenderBounds().height();
        if ((m_speed.y > 0) && (getPosition().y > m_bottom)) {
            setPosition(Vector(m_center.x, -PLATFORM_HEIGHT));
        } else if ((m_speed.y < 0) && (getPosition().y < 0)) {
            setPosition({ m_center.x, m_bottom + 16.f });
        }
        break;
    }
    case PlatformType::VERTICAL: // fall trough
    case PlatformType::HORIZONTAL: {
        int k = 1;
        if ((m_platform_type == PlatformType::HORIZONTAL && getPosition().x > m_center.x) ||
            (m_platform_type == PlatformType::VERTICAL && getPosition().y > m_center.y)) {
            k = -1;
        }
        m_timer += delta_time;
        m_speed += k * m_orientation * m_acceleration * delta_time;
        break;
    }
    }
 
    move(m_speed * delta_time);
}

Vector MoveablePlatform::getSpeedVector() {
    return 2 * m_speed;
}

void MoveablePlatform::onStarted() {
    const int height = 16;
    m_mario = getParent()->findChildObjectByType<Mario>();

    // read properties
    m_platform_type = static_cast<PlatformType>(getProperty("Orientation").asInt());
    m_amplitude = getProperty("Amplitude").asInt();
    m_period_time = getProperty("Period").asFloat() * 1000;
    m_size = getProperty("width").asFloat();

    if (m_platform_type == PlatformType::UNKNOWN) {
        return;
    }

    // prepare sprite
    m_sprite.setTextureRect({{0,0}, {m_size,height}});
    setSize(Vector(m_size, height));
 
    m_timer = 0;// m_amplitude + getProperty("Phase").asInt() / (m_max_speed*speed_koef);
    m_center = getPosition();

    switch (m_platform_type) {
    case PlatformType::ELEVATOR:
        m_bottom = getParent()->findChildObjectByType<Blocks>()->getRenderBounds().height();
        m_speed.y = m_bottom / m_period_time;
        break;
    case PlatformType::HORIZONTAL: // fall trough
    case PlatformType::VERTICAL:
        m_orientation = (m_platform_type == PlatformType::HORIZONTAL) ? Vector::RIGHT
                                                                      : Vector::DOWN;
        m_acceleration = 2 * (m_amplitude) / pow(m_period_time * 0.25f, 2);
        m_speed = m_acceleration * (m_period_time * 0.25f) * m_orientation;
        setPosition(m_center);
        break;
    case PlatformType::SKATE:
        m_sprite.setTextureRect({{112, 0}, {m_size,height}});
        m_orientation = Vector::RIGHT;
        m_speed = Vector::ZERO;
        break;
    }

    if (getProperty("Phase").asInt() != 0) {
        m_speed = -m_speed;
    }
}

void MoveablePlatform::collsionResponse(Mario* mario, ECollisionTag& collision_tag, int delta_time) {
    Platform::collsionResponse(mario, collision_tag, delta_time);

    if ((collision_tag & ECollisionTag::FLOOR) &&
        (m_platform_type == PlatformType::SKATE) &&
        (m_speed == Vector::ZERO)) {
        // activate skate
        m_speed = Vector::RIGHT * SKATE_SPEED;
    }
}
//---------------------------------------------------------------------------
//! FallingPlatform
//---------------------------------------------------------------------------
FallingPlatform::FallingPlatform()
    : m_sprite(*MARIO_GAME.textureManager().get("Items"),
        { {0,0}, {1,1}})
{
}

void FallingPlatform::draw(sf::RenderWindow* render_window) {
    m_sprite.setPosition(getPosition());
    render_window->draw(m_sprite);
}

void FallingPlatform::setSpeed(const Vector& speed) {
    m_speed = speed;
}

void FallingPlatform::update(int delta_time) {
    if (m_stayed) {
        m_speed += Vector::DOWN * delta_time * 0.008f;
        m_stayed = false;

        if (m_moving_callback)
        {
            m_moving_callback();
        }
    } else {
        m_speed += Vector::UP * math::sign(m_speed.y) * 0.001f * delta_time;
        if (std::abs(m_speed.y < 0.05)) {
            m_speed = Vector::ZERO;
        }
    }

    move(m_speed * delta_time);
}

void FallingPlatform::collsionResponse(Mario* mario, ECollisionTag& collision_tag, int delta_time) {
    Platform::collsionResponse(mario, collision_tag, delta_time);
    if (collision_tag & ECollisionTag::FLOOR) {
        m_stayed = true;
        m_mario = mario;
    }
}

Vector FallingPlatform::getSpeedVector() {
    return m_speed;
}

void FallingPlatform::onStarted() {
    Vector size(getProperty("width").asFloat(), 16.f);

    setBounds(Rect(getPosition(), size));

    m_sprite.setTextureRect({ {0,0}, {(int)size.x, (int)size.y }});
}

void FallingPlatform::addImpulse(const Vector& speed) {
    m_speed += speed;
}

void FallingPlatform::setMovingCallback(const std::function<void()>& func) {
    m_moving_callback = func;
}
//---------------------------------------------------------------------------
//! PlatformSystem
//---------------------------------------------------------------------------
PlatformSystem::PlatformSystem() {
    auto texture = MARIO_GAME.textureManager().get("Items");

    m_sprite_sheet.load(*texture, {
        { {100, 16}, {32,  32}},  // lNode
        { {132, 16}, {-32, 32}},  // rNode
        { {132, 16}, {32,  32}},  // H-line
        { {100, 48}, {32,  32}},  // lV-line
        { {132, 48}, {-32, 32}}   // rV-line
     });
}

void PlatformSystem::onStarted() {
    setSize({ getProperty("width").asFloat(), getProperty("height").asFloat() });
    m_left_platform = new FallingPlatform();
    m_right_platform = new FallingPlatform();

    float width = getBounds().width() / 3;
    m_left_platform->setProperty("width", Property(width));
    m_right_platform->setProperty("width", Property(width));
    m_left_platform->setPosition(getBounds().leftBottom()  + Vector(0, PLATFORM_HEIGHT));
    m_right_platform->setPosition(getBounds().leftBottom() + Vector(width * 2, PLATFORM_HEIGHT));

    m_left_platform->setMovingCallback(std::bind(&PlatformSystem::onLeftPlatformMove, this));
    m_right_platform->setMovingCallback(std::bind(&PlatformSystem::onRightPlatformMove, this));
    addChild(m_left_platform);
    addChild(m_right_platform);
}

void PlatformSystem::draw(sf::RenderWindow* render_window) {
    GameObject::draw(render_window);

    int WIDTH = getBounds().width();
    int width = getBounds().width() / 6;
    int left_shoulder  = m_left_platform->getPosition().y  - getPosition().y - 32;
    int right_shoulder = m_right_platform->getPosition().y - getPosition().y - 32;

    m_sprite_sheet.setPosition(getPosition() + Vector(width - 16, 0));
    m_sprite_sheet.draw(render_window, 0);

    m_sprite_sheet.setPosition(getPosition() + Vector(WIDTH - width - 16, 0));
    m_sprite_sheet.draw(render_window, 1);

    m_sprite_sheet.draw(render_window, 2, Rect(width + 16, 0, WIDTH * 2 / 3 - 32, 32).moved(getPosition()), RepeatMode::REPEAT);
    m_sprite_sheet.draw(render_window, 3, Rect(width - 16, 32, 32, left_shoulder).moved(getPosition()), RepeatMode::REPEAT);
    m_sprite_sheet.draw(render_window, 4, Rect(WIDTH - 16 - width, 32, 32, right_shoulder).moved(getPosition()), RepeatMode::REPEAT);
}

void PlatformSystem::update(int delta_time) {
    GameObject::update(delta_time);
}

void PlatformSystem::onLeftPlatformMove() {
    if (m_left_platform->getPosition().y - getPosition().y > 2*getBounds().height() - 32) {
        m_left_platform->setSpeed(Vector::ZERO);
    } else {
        float wr = 2 * (getBounds().height() + getPosition().y) - m_left_platform->getPosition().y;
        m_right_platform->setPosition(m_right_platform->getPosition().x, + wr);
    }
}

void PlatformSystem::onRightPlatformMove() {
    if (m_right_platform->getPosition().y - getPosition().y > 2*getBounds().height() - 32) {
        m_right_platform->setSpeed(Vector::ZERO);
    } else {
        float wl = 2 * (getBounds().height()+ getPosition().y) - m_right_platform->getPosition().y;
        m_left_platform->setPosition(m_left_platform->getPosition().x, + wl);
    }
}
//---------------------------------------------------------------------------
//! Jumper
//---------------------------------------------------------------------------
Jumper::Jumper() {
    auto& texture = *MARIO_GAME.textureManager().get("Items");
    m_animator.create("high",   texture, { 0,  20, 32, 64 });
    m_animator.create("middle", texture, { 32, 36, 32, 48 });
    m_animator.create("low",    texture, { 64, 52, 32, 32 });
}

void Jumper::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

void Jumper::update(int delta_time) {
    if (m_state == 4) {
        return;
    }

    m_timer += delta_time;
    int new_state = int(m_timer / 75) % 5;

    if (m_state == new_state) {
        return;
    }

    m_state = new_state;

    int height = 0;

    switch (m_state) {
    case 0: // fall trough
    case 4:
        m_animator.play("high");
        height = 64;
        break;
    case 1: // fall trough
    case 3:
        m_animator.play("middle");
        height = 48;
        break;
    case 2:
        if (m_mario) {
            m_mario->addImpulse(Vector::UP * 0.5);
        }
        m_animator.play("low");
        height = 32;
        break;
    }

    Rect old_bounds = getBounds();
    old_bounds.setTop(m_bottom - height);
    setBounds(old_bounds);
}

void Jumper::collsionResponse(Mario* mario, ECollisionTag& collision_tag, int delta_time) {
    if (mario->getSpeed().y >= 0) {
        mario->setPosition(::collisionResponse(mario->getBounds(), mario->getSpeed(), getBounds(),
                           Vector::ZERO, delta_time, collision_tag));
        mario->setSpeed({ mario->getSpeed().x, 0.f});
    }

    if (collision_tag == ECollisionTag::FLOOR) {
        m_mario = mario;

        if (m_state == 4)
        {
            m_timer = 0;
            m_state = 0;
        }
    }
}

void Jumper::onStarted() {
    setSize({32, 64});
    m_bottom = getBounds().bottom();
}
//---------------------------------------------------------------------------
//! Ladder
//---------------------------------------------------------------------------
Ladder::Ladder() 
    : m_sprite(*MARIO_GAME.textureManager().get("Items"), {{0,0}, {1,1}})
{
    setSize({ 10, 32 });
}

void Ladder::draw(sf::RenderWindow* render_window) {
    const int BLOCK_HEIGHT = 32;

    int k = getBounds().height() / BLOCK_HEIGHT;
    int remainder = int(getBounds().height()) % BLOCK_HEIGHT;

    int pos_x = getBounds().center().x - BLOCK_HEIGHT / 2;

    for (int i = 0; i <= k; ++i) {
        int top    = (i == 0) ? 0
                              : (2 - (i % 2)) * BLOCK_HEIGHT;
        int height = (i == k) ? remainder
                              : BLOCK_HEIGHT;

        m_sprite.setTextureRect({{212, top}, {BLOCK_HEIGHT, height}});
        m_sprite.setPosition(sf::Vector2f(pos_x, i * BLOCK_HEIGHT + getPosition().y));
        render_window->draw(m_sprite);
    }
}

void Ladder::update(int delta_time) {
    GameObject::update(delta_time);

    if (m_timer == 0) {
        MARIO_GAME.playSound("powerup_appears");
    }

    m_timer += delta_time;
    float cur_height = m_timer * 0.04f;

    if (cur_height < m_height) {
        setBounds({ getPosition().x, m_bottom - cur_height, m_width, cur_height + 32 });
    }
}

void Ladder::onStarted() {
    m_height = getPosition().y - getBounds().height() + 32;
    m_width = getBounds().width();
    m_bottom = getPosition().y;
    getParent()->findChildObjectByType<Mario>()->moveToFront();
}
//---------------------------------------------------------------------------
//! FireBar
//---------------------------------------------------------------------------
FireBar::FireBar() {
    m_animator.create("fly", *MARIO_GAME.textureManager().get("Mario"),
        { { {0,  0}, {16, 16 }},
          { {16, 0}, {16, 16 }},
          { {16, 0}, {-16, 16 }},
          { {16, 16}, {16, -16 }}
        }, 0.01f);
}

void FireBar::FireBar::draw(sf::RenderWindow* render_window) {
    for (const auto& fire_pos : m_fire_pos) {
        m_animator.setPosition(fire_pos);
        m_animator.draw(render_window);
    }
}

void FireBar::FireBar::update(int delta_time) {
    m_animator.update(delta_time);

    // rotation processing
    m_timer += delta_time;
    float angle = -m_speed* m_timer / 400;
    Vector rot((float)cos(angle), (float)sin(angle));

    for (size_t i=0; i < m_fire_pos.size(); ++i) {
        m_fire_pos[i] = getPosition() + rot * 16 * i - Vector(4,4);
    }

    // check collision with mario processing
    Rect own_bounds = Rect(m_fire_pos.front(), m_fire_pos.back() - m_fire_pos.front());
    own_bounds.normalize();
    auto mario_bounds = m_mario->getBounds();

    if (own_bounds.isIntersect(mario_bounds)) {
        for (const auto& fire_pos : m_fire_pos) {
            if (mario_bounds.isContain(fire_pos + Vector(8, 8))) {
                m_mario->takeDamage(DamageType::KICK, nullptr);
                break;
            }
        }
    }
}

void FireBar::onStarted() {
    int fires = getProperty("height").asFloat() / 16.f;
    m_speed = getProperty("Speed").asFloat();
    m_fire_pos.resize(fires);
    m_mario = getParent()->findChildObjectByType<Mario>();
}
//---------------------------------------------------------------------------
//! LevelPortal
//---------------------------------------------------------------------------
LevelPortal::LevelPortal() {
}

void LevelPortal::goToLevel() {
    MARIO_GAME.loadLevel(m_level_name);

    if (m_show_status) {
        MARIO_GAME.showStatus();
    }
}

void LevelPortal::goToSublevel() {
    if (m_came_back_portal) {
        m_mario->setUnclimb();
        m_mario->setPosition(m_came_back_portal->getBounds().center().x - m_mario->getBounds().width() / 2.f,
                             m_came_back_portal->getBounds().bottom() - m_mario->getBounds().height());

        if (m_came_back_portal->m_direction != Vector::ZERO) {
            m_mario->move(-m_came_back_portal->m_direction * TRANSITION_TIME * 0.03f);
            if (!m_mario->isSmall()) {
                m_mario->move(-m_came_back_portal->m_direction * 32.f);
            }
            m_mario->setState(new TransitionMarioState(m_came_back_portal->m_direction*0.03f, TRANSITION_TIME));
        }

        m_came_back_portal->m_used = true;
        getParent()->castTo<MarioGameScene>()->setCameraOnTarget();
    }

     MARIO_GAME.loadSubLevel(m_sub_level_name);
}

void LevelPortal::cameBackFromSublevel() {
    MARIO_GAME.unloadSubLevel();
}

void LevelPortal::update(int delta_time) {
    if (m_used) {
        return;
    }

    if (!getBounds().isContain(m_mario->getBounds())) {
        // not in the bounds
        return;
    }

    if ((m_direction == Vector::ZERO || m_mario->getInputDirection() == m_direction) &&
        (m_mario->isGrounded() || (m_direction == Vector::ZERO) || m_mario->isClimbing())) {
        m_used = true;
        if (m_direction != Vector::ZERO) {
            m_mario->setState(new TransitionMarioState(m_direction * 0.03f, TRANSITION_TIME));
        }

        switch (m_portal_type) {
        case PortalType::ENTER_LEVEL:
            MARIO_GAME.globalTimer().setTimer(this, &LevelPortal::goToLevel, TRANSITION_TIME);
            break;
        case PortalType::ENTER_SUBLEVEL:
            MARIO_GAME.globalTimer().setTimer(this, &LevelPortal::goToSublevel, (m_direction != Vector::ZERO) ? TRANSITION_TIME : 0);
            break;
        case PortalType::LEAVE_SUBLEVEL:
            MARIO_GAME.globalTimer().setTimer(this, &LevelPortal::cameBackFromSublevel, (m_direction != Vector::ZERO) ? TRANSITION_TIME : 0);
            break;
        }
    }
 }

void LevelPortal::onStarted() {
    setSize({ getProperty("width").asFloat(), getProperty("height").asFloat() });

    m_mario = getParent()->findChildObjectByType<Mario>();
    m_direction = Vector::fromString(getProperty("Direction").asString());

    if (getProperty("Level").isValid() && !getProperty("Level").asString().empty()) {
        m_level_name = getProperty("Level").asString();
        m_portal_type = PortalType::ENTER_LEVEL;
    } else if (getProperty("SubLevel").isValid() && !getProperty("SubLevel").asString().empty()) {
        m_sub_level_name = getProperty("SubLevel").asString();
        m_portal_type = PortalType::ENTER_SUBLEVEL;
        if (getProperty("ComebackPortal").isValid() && !getProperty("ComebackPortal").asString().empty()) {
            std::string came_back_portal_str = getProperty("ComebackPortal").asString();
            m_came_back_portal = getParent()->findChildObjectByName<LevelPortal>(came_back_portal_str);
            assert(m_came_back_portal); //no cameback portal findeded for sublevel portal
        }
    } else {
        m_portal_type = PortalType::LEAVE_SUBLEVEL;
    }

    if (getProperty("ShowStatus").isValid()) {
        m_show_status = getProperty("ShowStatus").asBool();
    }
}
//---------------------------------------------------------------------------
//! EndLevelFlag
//---------------------------------------------------------------------------
void EndLevelFlag::onStarted() {
    setSize({ getProperty("width").asFloat(), getProperty("height").asFloat() });
    m_mario = getParent()->findChildObjectByType<Mario>();

    auto m_block = getParent()->findChildObjectByType<Blocks>();
    m_cell_y = m_block->getRenderBounds().height() - m_block->blockSize().y * 4;
}

EndLevelFlag::EndLevelFlag() {
    m_animator.create("base", *MARIO_GAME.textureManager().get("Items"), { 0,180 }, { 32,32 }, 4, 1, 0.01f);
}

void EndLevelFlag::draw(sf::RenderWindow* render_window)  {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

void EndLevelFlag::update(int delta_time) {
    GameObject::update(delta_time);
    if (!m_touched) {
        if (m_mario->getBounds().right() > getPosition().x) {
            m_mario->setState(new GoToCastleMarioState());
            m_touched = true;
        }
    } else if (getPosition().y < m_cell_y) {
        move(Vector::DOWN * delta_time * FALLING_SPEED);
    }

    m_animator.update(delta_time);
}

EndLevelKey::EndLevelKey() 
    : m_sprite(*MARIO_GAME.textureManager().get("Items"), { {0,212}, {32,32} })
    {
    setSize({ 32, 32 });
}

void EndLevelKey::onStarted() {
    m_mario = getParent()->findChildObjectByType<Mario>();
    m_bowser = getParent()->findChildObjectByType<Bowser>();
    m_blocks = getParent()->findChildObjectByType<Blocks>();
}

void EndLevelKey::draw(sf::RenderWindow* render_window) {
    m_sprite.setPosition(getPosition());
    render_window->draw(m_sprite);
}

void EndLevelKey::enterState(State state) {
    m_state = state;

    switch (state) {
    case State::BRIDGE_HIDING:
        m_mario->disable();
        m_bowser->disable();
        MARIO_GAME.stopMusic();
        hide();
        m_bridge_blocks = m_blocks->getBridgeBlocks();
        m_delay_timer = 500;
        break;
    case State::BOWSER_RUN:
        m_bowser->enable();
        m_bowser->noBridge();
        m_delay_timer = 1000;
        break;
    case State::MARIO_GOING_TO_PRINCESS:
        m_mario->turnOn();
        m_mario->setState(new GoToPrincessState());
        break;
    case State::TEXT_SHOW:
        getParent()->addChild(MARIO_GAME.createText("Thank you Mario!",
                               getParent()->castTo<MarioGameScene>()->screenToPoint(MARIO_GAME.screenSize() / 2) - Vector(100, 100)));
        m_delay_timer = 1000;
        break;
    case State::GO_TO_NEXT_LEVEL:
        m_delay_timer = 5000;
        break;
    }
}

void EndLevelKey::update(int delta_time) {
    switch (m_state) {
    case State::PLAY:
        if (m_mario->getBounds().isIntersect(getBounds())) {
            if (getParent()->findChildObjectByName("Bowser")) {
                enterState(State::BRIDGE_HIDING);
            } else {
                MARIO_GAME.stopMusic();
                MARIO_GAME.playSound("world_clear");
                hide();
                enterState(State::MARIO_GOING_TO_PRINCESS);
            }
        }
        break;
    case State::BRIDGE_HIDING:
        m_delay_timer -= delta_time;
        if (m_delay_timer < 0) {
            m_delay_timer = 100;
            Vector block = m_bridge_blocks.back();
            m_blocks->clearBlock(block.x, block.y);
            m_bridge_blocks.pop_back();
            if (m_bridge_blocks.empty()) {
                enterState(State::BOWSER_RUN);
            }
            MARIO_GAME.playSound("breakblock");
        }
        break;
    case State::BOWSER_RUN:
        m_delay_timer -= delta_time;
        if (m_delay_timer < 0) {
            enterState(State::MARIO_GOING_TO_PRINCESS);
            MARIO_GAME.playSound("world_clear");
        }
        break;
    case State::MARIO_GOING_TO_PRINCESS:
        if (m_mario->getSpeed().x == 0)
        {
            enterState(State::TEXT_SHOW);
        }
        break;
    case State::TEXT_SHOW:
        m_delay_timer -= delta_time;
        if (m_delay_timer < 0) {
            Vector pos = getParent()->castTo<MarioGameScene>()->screenToPoint(MARIO_GAME.screenSize() / 2) - Vector(120, 40);
            getParent()->addChild(MARIO_GAME.createText("But our princess is in\n another castle!", pos));
            enterState(State::GO_TO_NEXT_LEVEL);
        }
        break;
    case State::GO_TO_NEXT_LEVEL:
        m_delay_timer -= delta_time;
        if (m_delay_timer < 0) {
            LevelPortal* portal = getParent()->findChildObjectByType<LevelPortal>();
            assert(portal->getProperty("Level").isValid());
            const std::string& next_level =  portal->getProperty("Level").asString();
            MARIO_GAME.loadLevel(next_level);
            MARIO_GAME.showStatus();
        }
        break;
    }
}
//---------------------------------------------------------------------------
//! CastleFlag
//---------------------------------------------------------------------------
CastleFlag::CastleFlag() {
    m_animator.create("normal", *MARIO_GAME.textureManager().get("Items"),
                      {0, 148}, {32, 32}, 4, 1, 0.01f);
    setSize({ 32,32 });
}

void CastleFlag::onStarted() {
    move(Vector::DOWN * 64); // hide for start
    m_pos_y = getPosition().y;
    moveUnderTo(getParent()->findChildObjectByType<Blocks>());
    disable();
}

void CastleFlag::liftUp() {
    enable();
    m_pos_y -= 64;
}

void CastleFlag::draw(sf::RenderWindow* render_window)
{
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

void CastleFlag::update(int delta_time) {
    if (getPosition().y > m_pos_y) {
        move(Vector::UP * delta_time * 0.03f);
    }

    m_animator.update(delta_time);
}

Princess::Princess() {
    setSize({ 32,64 });
    m_animator.create("stay", *MARIO_GAME.textureManager().get("Items"), { 222,96,32,64 });
}

void Princess::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}
//---------------------------------------------------------------------------
// ! Trigger
//---------------------------------------------------------------------------
void Trigger::onStarted() {
    m_mario = getParent()->findChildObjectByType<Mario>();
    setSize({ getProperty("width").asFloat(),
              getProperty("height").asFloat() });
}

void Trigger::update(int delta_time) {
    if (!m_trigered && getBounds().isContain(m_mario->getBounds())) {
        m_trigered = true;

        if (getProperty("EnableAction").isValid()) {
            auto object_names = utils::split(getProperty("EnableAction").asString(), ';');
            for (auto& object_name : object_names) {
                auto object = getParent()->findChildObjectByName(object_name);
                if (object) {
                    object->show();
                }
            }
        }
    }
}
