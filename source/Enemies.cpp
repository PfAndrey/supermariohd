#include <cmath>

#include <Logger.hpp>
#include "Blocks.hpp"
#include "Enemies.hpp"
#include "Mario.hpp"
#include "SuperMarioGame.hpp"

namespace {

bool isCharacterInFront(Character* target, GameObject* origin) {
    return (target->getBounds().center().x > origin->getBounds().center().x);
}

} // anonymous namespace

void Enemy::checkNextTileUnderFoots() {
    if (m_speed.y != 0) {
        return;
    }

    Vector own_center = getBounds().center();
    Vector opposite_vector = math::sign(m_speed.x) * Vector::RIGHT;
    bool is_next_under_foot = m_blocks->isCollidableBlock(m_blocks->toBlockCoordinates(own_center + 20*opposite_vector + 32*Vector::DOWN));
    bool is_prev_under_foot = m_blocks->isCollidableBlock(m_blocks->toBlockCoordinates(own_center - 60*opposite_vector + 32*Vector::DOWN));
    bool is_prev_back = m_blocks->isCollidableBlock(m_blocks->toBlockCoordinates(own_center - 50*opposite_vector));
    bool is_next_back = m_blocks->isCollidableBlock(m_blocks->toBlockCoordinates(own_center + 50*opposite_vector));

    if ((!is_next_under_foot && !is_prev_back) && (is_next_under_foot || is_prev_under_foot)) {
            m_speed.x = -m_speed.x;
    }
}

void Enemy::checkCollideOtherCharasters()
{
    auto enemies = getParent()->findChildObjectsByType<Enemy>();
    for (auto enemy : enemies) {
        if ((enemy != this) && enemy->isAlive() && enemy->getBounds().isIntersect(getBounds())) {
            enemy->takeDamage(DamageType::HIT_FROM_BELOW, this);
        }
    }
}

void Enemy::updateCollision(float delta_time) {
    m_collision_tag = ECollisionTag::NONE;
    setPosition(m_blocks->collsionResponse(getBounds(), m_speed, delta_time, m_collision_tag));
}

void Enemy::updatePhysics(float delta_time, float gravity) {
    m_speed += Vector::DOWN * gravity * delta_time;
    move(delta_time * m_speed);
}

bool Enemy::isInCamera() const {
    Rect cameraRect = getParent()->castTo<MarioGameScene>()->cameraRect();
    return (std::abs(getPosition().x - cameraRect.center().x) < cameraRect.width() / 2);
}

void Enemy::playAnimation(const std::string& name) {
    m_animator.play(name);
}

void Enemy::checkFallUndergound() {
    if (getPosition().y > 1000) {
        removeLater();
    }
}

Mario* Enemy::mario() {
    return m_mario;
}

void Enemy::addScoreToPlayer(int score) {
    MARIO_GAME.addScore(score, getBounds().center());
}

void Enemy::onStarted() {
    m_mario = MARIO_GAME.getPlayer();
    m_blocks = getParent()->findChildObjectByType<Blocks>();
    assert(m_mario && m_blocks);
}

void Enemy::update(int delta_time) {
    checkFallUndergound();
}
//---------------------------------------------------------------------------
//! Goomba
//---------------------------------------------------------------------------
Goomba::Goomba() {
    setSize({ 32,32 });
    const sf::Texture& texture = *MARIO_GAME.textureManager().get("Enemies");
    m_animator.create("walk",    texture, { { 0, 0, 32, 32 },{ 32, 0, 32, 32 } }, 0.005f);
    m_animator.create("cramped", texture, { 64, 0, 32, 32 });
    m_animator.create("fall",    texture, { 0, 32, 32, -32 });
    m_animator.setSpriteOffset("cramped", 0, { 0,8 });
}

void Goomba::setState(State state) {
    if (m_state == state) {
        return;
    }
 
    m_state = state;

    switch (m_state) {
    case State::NORMAL:
        m_speed.x = RUN_SPEED;
        playAnimation("walk");
        break;
    case State::CRAMPED:
        m_speed.x = 0;
        playAnimation("cramped");
        addScoreToPlayer(100);
        MARIO_GAME.playSound("stomp");
        break;
    case State::DIED:
        m_speed.x = 0;
        m_speed += 0.4f * Vector::UP;
        playAnimation("fall");
        addScoreToPlayer(100);
        MARIO_GAME.playSound("kick");
        break;
    }

    m_timer = 0;
}

void Goomba::update(int delta_time) {
    Enemy::update(delta_time);

    switch (m_state) {
    case State::DEACTIVATED:
        if (isInCamera()) {
            setState(State::NORMAL);
        }
        break;
    case State::NORMAL:
        updatePhysics(delta_time, GRAVITY_FORCE);
        updateCollision(delta_time);
        if (m_collision_tag & ECollisionTag::Y_AXIS) {
            m_speed.y = 0;
        }
        if (m_collision_tag & ECollisionTag::X_AXIS) {
            m_speed.x = -m_speed.x;
        }
        m_animator.update(delta_time);
        break;
    case State::DIED:
        updatePhysics(delta_time, GRAVITY_FORCE);
        break;
    case State::CRAMPED:
        m_timer += delta_time;
        if (m_timer > 3000) {
            removeLater();
        }
        break;
    }
}

void Goomba::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

void Goomba::takeDamage(DamageType damageType, Character*) {
    if (damageType == DamageType::HIT_FROM_ABOVE) {
        setState(State::CRAMPED);
    } else {
        setState(State::DIED);
    }
}

void Goomba::touch(Character* character) {
    character->takeDamage(DamageType::KICK, this);
}

bool Goomba::isAlive() const {
    return (m_state == State::NORMAL);
}
//---------------------------------------------------------------------------
//! Koopa
//---------------------------------------------------------------------------
Koopa::Koopa() {
    setSize({ 32, 48 });
    const sf::Texture& texture = *MARIO_GAME.textureManager().get("Enemies");
    m_animator.create("walk",   texture, { { 0,32,32,48 },{ 32,32,32,48 } }, 0.005f);
    m_animator.create("flying", texture, { 224,32 }, { 32, 48 }, 2, 1, 0.005f);
    m_animator.create("climb",  texture, { { 64,48,32,32 },{ 192,48,32,32 } }, 0.005f);
    m_animator.create("hidden", texture, { 64,48,32,32 });
    m_animator.create("bullet", texture, { 64, 48 }, { 32, 32 }, 4, 1, 0.01f);
    m_animator.create("fall",   texture, { 0,80, 32, -48 });
}

void Koopa::setState(State state) {
    if (m_state == state) {
        return;
    }

    m_state = state;

    switch (m_state) {
    case State::JUMPING:
        m_speed.x = RUN_SPEED;
        playAnimation("flying");
        break;
    case State::LEVITATING:
        playAnimation("flying");
        m_speed.x = 0;
        m_initial_pos = getPosition();
        break;
    case State::NORMAL:
        m_speed.x = RUN_SPEED;
        m_animator.flipX(m_speed.x > 0);
        playAnimation("walk");
        move(getBounds().size() - full_size);
        setSize(full_size);
        break;
    case State::HIDDEN:
        move(getBounds().size() - hidden_size);
        setSize(hidden_size);
        m_speed.x = 0;
        playAnimation("hidden");
        break;
    case State::BULLET:
        move(getBounds().size() - hidden_size);
        setSize(hidden_size);
        playAnimation("bullet");
        break;
    case State::CLIMB:
        move(getBounds().size() - hidden_size);
        setSize(hidden_size);
        playAnimation("climb");
        break;
    case State::DIED:
        m_speed.y = -0.4f;
        playAnimation("fall");
        MARIO_GAME.playSound("kick");
        break;
    }

    m_timer = 0;
}

void Koopa::update(int delta_time) {
    Enemy::update(delta_time);
 
    if ((m_state != State::DIED) &&
        (m_state != State::LEVITATING) &&
        (m_state != State::DEACTIVATED)) {
        updatePhysics(delta_time, (m_state != State::JUMPING) ? GRAVITY_FORCE : 0.5 * GRAVITY_FORCE);
        updateCollision(delta_time);

        if (m_collision_tag & ECollisionTag::X_AXIS) {
            m_speed.x = -m_speed.x;
        }

        if (m_collision_tag & ECollisionTag::Y_AXIS) {
            m_speed.y = 0;
        }

        m_animator.update(delta_time);
        m_animator.flipX(m_speed.x > 0);
    }
 
    if (m_state == State::LEVITATING) {
        m_animator.update(delta_time);
        m_animator.flipX(m_speed.x > 0);
    }

    switch (m_state) {
    case State::DEACTIVATED:
        if (isInCamera()) {
            setState(m_initial_state);
        }
        break;
    case State::NORMAL:
        checkNextTileUnderFoots();
        break;
    case State::JUMPING:
        if (m_collision_tag & ECollisionTag::FLOOR) {
            m_speed.y = -0.4f; // jump
        }
        break;
    case State::LEVITATING:
        m_timer += delta_time;
        setPosition(m_initial_pos + Vector::UP * sin(m_timer / 800.f) * 50);
        break;
    case State::HIDDEN:
        m_timer += delta_time;
        if (m_timer > 3000) {
            setState(State::CLIMB);
        }
        break;
    case State::CLIMB:
        m_timer += delta_time;
        if (m_timer > 5000) {
            setState(State::NORMAL);
        }
        break;
    case State::BULLET:
        checkCollideOtherCharasters();
        if (m_collision_tag & ECollisionTag::X_AXIS) {
            getParent()->castTo<MarioGameScene>()->playSoundAtPoint("bump", getBounds().center());
        }
        break;
    case State::DIED:
        updatePhysics(delta_time, GRAVITY_FORCE);
        break;
    }
}

void Koopa::takeDamage(DamageType damageType, Character* attacker) {
    if (!isAlive()) {
        return;
    }

    if (damageType == DamageType::HIT_FROM_ABOVE) {
        switch (m_state) {
        case State::LEVITATING: // fall through
        case State::JUMPING:
            m_speed.y = 0;
            setState(State::NORMAL);
            addScoreToPlayer(100);
            MARIO_GAME.playSound("stomp");
            break;
        case State::NORMAL:
            if (m_collision_tag & ECollisionTag::FLOOR) {
                setState(State::HIDDEN);
                addScoreToPlayer(100);
                MARIO_GAME.playSound("stomp");
            }
            break;
        case State::HIDDEN: // fall through
        case State::CLIMB:
            setState(State::BULLET);
            m_speed.x = isCharacterInFront(mario(), this) ? -std::abs(RUN_SPEED) * 6
                                                          : std::abs(RUN_SPEED) * 6;
            addScoreToPlayer(400);
            MARIO_GAME.playSound("kick");
            break;
        case State::BULLET:
            setState(State::HIDDEN);
            break;
        }
    } else {
       //  Handle case when other (Koopa or BuzzyBeetle) hit this Koopa from below (bullet state)
       //  if ((damageType == DamageType::HIT_FROM_BELOW) && attacker && attacker->isTypeOf<Enemy>())
       //  {
       //     attacker->takeDamage(DamageType::HIT_FROM_BELOW, nullptr);
       //  }

        setState(State::DIED);
        addScoreToPlayer(500);
        MARIO_GAME.playSound("kick");
    }
}

void Koopa::touch(Character* character) {
    switch (m_state) {
    case State::HIDDEN: // fall through
    case State::CLIMB: 
        setState(State::BULLET);
        m_speed.x = isCharacterInFront(character, this) ? -std::abs(RUN_SPEED) * 6
                                                        : std::abs(RUN_SPEED) * 6;
        move(15 * Vector::RIGHT * math::sign(m_speed.x));
        addScoreToPlayer(400);
        MARIO_GAME.playSound("kick");
        break;
    case State::BULLET: // fall through
    case State::NORMAL:
        character->takeDamage(DamageType::KICK, this);
        break;
    }
}

void Koopa::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

bool Koopa::isAlive() const {
    return (m_state != State::DIED);
}

void Koopa::onStarted() {
    Enemy::onStarted();

    switch(getProperty("Flying").asInt()) {
    case 1:
        m_initial_state = State::JUMPING;
        break;
    case 2:
        m_initial_state = State::LEVITATING;
        break;
    default:
        m_initial_state = State::NORMAL;
        break;
    }
}
//---------------------------------------------------------------------------
//! BuzzyBeetle
//---------------------------------------------------------------------------
BuzzyBeetle::BuzzyBeetle() {
    setSize({ 32, 32 });
    const sf::Texture& texture = *MARIO_GAME.textureManager().get("Enemies");
    m_animator.create("walk", texture, { { 96,0,32,32 },{ 128,0,32,32 } }, 0.005f);
    m_animator.create("hidden", texture, { 160,0,32,32 });
    m_animator.create("bullet", texture, { 160, 0 }, { 32, 32 }, 4, 1, 0.01f);
    m_animator.create("fall", texture, { 96,32, 32, -32 });
    setState(State::NORMAL);
}

void BuzzyBeetle::takeDamage(DamageType damageType, Character* attacker) {
    if (!isAlive()) {
        return;
    }

    m_timer = 0;

    if (damageType == DamageType::HIT_FROM_ABOVE) {
        switch (m_state) {
        case State::NORMAL:
            if (m_speed.y == 0) {
                setState(State::HIDDEN);
                addScoreToPlayer(100);
                MARIO_GAME.playSound("stomp");
            }
        case State::HIDDEN:
            setState(State::BULLET);
            
            m_speed.x = isCharacterInFront(attacker, this) ? -std::abs(RUN_SPEED) * 6
                                                           : std::abs(RUN_SPEED) * 6;
            addScoreToPlayer(400);
            MARIO_GAME.playSound("kick");
        case State::BULLET:
            setState(State::HIDDEN);
            break;
        }
    } else if (damageType == DamageType::HIT_FROM_BELOW) {
        // Handle case when other (Koopa or BuzzyBeetle) hit this BuzzyBeetle from below (bullet state)
        if ((damageType == DamageType::HIT_FROM_BELOW) && attacker && attacker->isAlive() && attacker->isTypeOf<Enemy>()) {
            attacker->takeDamage(DamageType::HIT_FROM_BELOW, nullptr);
        }

        setState(State::DIED);
        m_speed.y = -0.4f;
        addScoreToPlayer(800);
        MARIO_GAME.playSound("kick");
    }
}

void BuzzyBeetle::touch(Character* character) {
    switch(m_state)
    {
    case State::HIDDEN:
        setState(State::BULLET);
        m_speed.x = isCharacterInFront(character, this) ? -std::abs(RUN_SPEED) * 6
                                                        : std::abs(RUN_SPEED) * 6;
        move(14 * Vector::RIGHT * math::sign(m_speed.x));
        MARIO_GAME.playSound("kick");
        break;
    case State::NORMAL: // fall through
    case State::BULLET:
        character->takeDamage(DamageType::KICK, this);
        break;
    }
}

bool BuzzyBeetle::isAlive() const {
    return (m_state != State::DIED);
}

void BuzzyBeetle::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

void BuzzyBeetle::setState(State state) {
    m_state = state;
    m_timer = 0;

    switch (m_state) {
    case State::NORMAL:
        m_speed.x = RUN_SPEED;
        playAnimation("walk");
        break;
    case State::HIDDEN:
        m_speed.x = 0;
        playAnimation("hidden");
        break;
    case State::BULLET:
        m_speed.x = 6 * RUN_SPEED;
        playAnimation("bullet");
        break;
    case State::DIED:
        m_speed.y = -0.4f;
        playAnimation("fall");
        break;
    }
}

void BuzzyBeetle::update(int delta_time) {
    Enemy::update(delta_time);

    if ((m_state != State::DIED) &&
        (m_state != State::DEACTIVATED))
    {
        updatePhysics(delta_time, GRAVITY_FORCE);
        updateCollision(delta_time);

        if (m_collision_tag & ECollisionTag::X_AXIS) {
            m_speed.x = -m_speed.x;
        }

        if (m_collision_tag & ECollisionTag::Y_AXIS) {
            m_speed.y = 0;
        }

        m_animator.update(delta_time);
        m_animator.flipX(m_speed.x > 0);
    }

    switch (m_state) {
    case State::DEACTIVATED:
        if (isInCamera()) {
            setState(State::NORMAL);
        }
        break;
    case State::NORMAL:
        checkNextTileUnderFoots();
        break;
    case State::HIDDEN:
        m_timer += delta_time;
        if (m_timer > 5000) {
            setState(State::NORMAL);
            m_timer = 0;
        }
        break;
    case State::BULLET:
        checkCollideOtherCharasters();
        if (m_collision_tag & ECollisionTag::X_AXIS) {
            getParent()->castTo<MarioGameScene>()->playSoundAtPoint("bump", getBounds().center());
        }
        break;
    case State::DIED:
        updatePhysics(delta_time, GRAVITY_FORCE);
        break;
    }

    m_animator.flipX(m_speed.x > 0);
    m_animator.update(delta_time);
}
//---------------------------------------------------------------------------
//! Hammer
//---------------------------------------------------------------------------
Hammer::Hammer(Mario* target) {
    const sf::Texture& texture = *MARIO_GAME.textureManager().get("Enemies");
    m_animator.create("fly", texture, Vector(96, 112), Vector(32, 32), 4, 1, 0.01f);
    m_animator.create("in_hand", texture, { 96,112,32,32 });
    m_animator.play("in_hand");
    m_target = target;
    setSize({ 32,32 });
}

void Hammer::update(int delta_time) {
    if (m_state == State::FLY) {
        m_animator.update(delta_time);
        m_speed += Vector::DOWN * GRAVITY_FORCE * delta_time;
        move(m_speed*delta_time);

        if (getPosition().y > 1000) {
            removeLater();
        }

        if (m_target->getBounds().isContain(getBounds().center())) {
            m_target->takeDamage(DamageType::SHOOT, nullptr);
        }
    }
};

void Hammer::throwAway(const Vector& speed) {
    m_speed = speed;
    m_animator.play("fly");
    m_state = State::FLY;
}

void Hammer::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}
//---------------------------------------------------------------------------
//! HammerBro
//---------------------------------------------------------------------------
HammerBro::HammerBro() {
    setSize({ 32, 44 });
    const sf::Texture& texture = *MARIO_GAME.textureManager().get("Enemies");
    m_animator.create("died", texture, { 96,160 + 48,32,-48 });
    m_animator.create("walk", texture, Vector(96, 160), Vector(32, 48), 2, 1, 0.005f);
    m_animator.create("walk_with_hammer", texture, Vector(160, 160), Vector(32, 48), 2, 1, 0.005f);
    m_animator.play("walk_with_hammer");
    m_speed.x = RUN_SPEED;
}

void HammerBro::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

void HammerBro::update(int delta_time) {
    Enemy::update(delta_time);

    if (m_speed.y == 0) {
        m_animator.update(delta_time);
    }

    switch (m_state) {
    case State::NORMAL:
        // Physic processing
        updatePhysics(delta_time, GRAVITY_FORCE / 2);

        if (m_collision_on) {
            updateCollision(delta_time);
 
            if (m_collision_tag & ECollisionTag::X_AXIS) {
                m_speed.x = -m_speed.x;
            }
            if (m_collision_tag & ECollisionTag::Y_AXIS) {
                m_speed.y = 0;
            }
        }

        // Walk processing
        if (std::abs(getPosition().x - m_center_x) > WALK_AMPLITUDE) {
            m_speed.x = -m_speed.x;
        }

        m_direction = (mario()->getPosition().x > getPosition().x) ? Vector::RIGHT
                                                                   : Vector::LEFT;
        m_animator.flipX(m_direction == Vector::RIGHT);

        // Jump processing
        m_jump_timer += delta_time;
        if (m_jump_timer > JUMP_RATE) {
            if (m_jump_direction == Vector::UP && !isCanJumpUp() && isCanJumpDown())
                m_jump_direction = Vector::DOWN;
            if (m_jump_direction == Vector::DOWN && !isCanJumpDown())
                m_jump_direction = Vector::UP;

            if (m_jump_direction == Vector::UP) {   // jump up
                m_speed += Vector::UP * 0.5;
            } else {                                  // jump-off down
                m_speed += Vector::UP * 0.25;
                m_drop_off_height = getPosition().y + getBounds().height() + 32.f;
            }

            m_collision_on = false;                // turn off collision check for moving through walls
            m_jump_timer = 0;
        }

        if (!m_collision_on) {                      // turn on collision check for take ground
            if (m_jump_direction == Vector::UP) {
                m_collision_on = (m_speed.y > 0);
            } else {
                m_collision_on = (getPosition().y > m_drop_off_height);
            }
        }

        // Fire processing
        m_fire_timer += delta_time;
        if (m_fire_timer < FIRE_RATE / 2) {
            // just walk
        } else if (m_fire_timer < FIRE_RATE)  {
            // get hammer in hand
            if (!m_hummer) {
                m_hummer = MARIO_GAME.spawnObject<Hammer>(mario());
                playAnimation("walk_with_hammer");
            }
            const Vector hand_off_set = { -3 * m_direction.x, -22.f };
            m_hummer->setPosition(getPosition() + hand_off_set);
        } else {
            // throw hummer
            m_hummer->throwAway({ m_direction.x * 0.15f, -0.55f });
            m_hummer = nullptr;
            m_fire_timer = 0;
            playAnimation("walk");
        };
        break;
    case State::DIED:
        updatePhysics(delta_time, GRAVITY_FORCE);
        break;
    }
}

void HammerBro::setState(State state) {
    m_state = state;
    if (m_state == State::DIED) {
        m_speed.y = 0;
        if (m_hummer) {
            m_hummer->removeLater();
            m_hummer = nullptr;
        }
        playAnimation("died");
        MARIO_GAME.playSound("kick");
        addScoreToPlayer(1000);
    }
}

void HammerBro::onStarted() {
    Enemy::onStarted();
    m_center_x = getPosition().x;
    m_fire_timer = rand() % 500;
    m_jump_timer = rand() % int(JUMP_RATE / 2);
}

void HammerBro::takeDamage(DamageType damageType, Character* attacker) {
    setState(State::DIED);
}

void HammerBro::touch(Character* character) {
    if (m_state == State::NORMAL) {
        character->takeDamage(DamageType::KICK, this);
    }
}

bool HammerBro::isAlive() const {
    return (m_state != State::DIED);
}

bool HammerBro::isCanJumpUp() const {
    Vector begin_point = m_blocks->toBlockCoordinates(getBounds().center());
    Vector end_point = begin_point;

    while (m_blocks->isBlockInBounds(end_point) && !m_blocks->isCollidableBlock(end_point)) {
        end_point += Vector::UP;
    }

    if ((end_point == begin_point) || (end_point.y <= 0)) {
        return false;
    }

    bool result = (std::abs(end_point.y - begin_point.y) >= 2); 
    return result;
}

bool HammerBro::isCanJumpDown() const {
    Vector begin_point = m_blocks->toBlockCoordinates(getBounds().center()) + Vector::DOWN * 2;
    return (!m_blocks->isCollidableBlock(begin_point));
}
//---------------------------------------------------------------------------
//! Spinny
//---------------------------------------------------------------------------
Spinny::Spinny(const Vector& position, const Vector& speed, const Vector& walk_direction) {
    setPosition(position);
    m_speed = speed;
    m_walk_direction = walk_direction;
    setSize({ 31, 32 });

    const auto& texture = *MARIO_GAME.textureManager().get("Enemies");
    m_animator.create("walk", texture, Vector(64, 80), Vector(32, 32), 2, 1, 0.005f);
    m_animator.create("egg", texture, Vector(128, 80), Vector(32, 32), 2, 1, 0.005f);
    m_animator.create("died", texture, { 64,80 + 32,32,-32 });

    setState(State::EGG);
}

void Spinny::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

void Spinny::setState(State state) {
    m_state = state;

    switch(m_state) {
    case State::NORMAL:
        playAnimation("walk");
        m_speed.x = RUN_SPEED;
        if (m_walk_direction == Vector::RIGHT) {
            m_speed = -m_speed;
        }
        break;
    case State::DIED:
        playAnimation("died");
        addScoreToPlayer(400);
        MARIO_GAME.playSound("kick");
        break;
    case State::EGG:
        playAnimation("egg");
        break;
    }
}

void Spinny::update(int delta_time) {
    Enemy::update(delta_time);
    m_animator.update(delta_time);

    if (m_state != State::DIED) {
        updatePhysics(delta_time, GRAVITY_FORCE / 2);
        updateCollision(delta_time);

        if (m_collision_tag & ECollisionTag::X_AXIS) {
            m_speed.x = -m_speed.x;
        }
        if (m_collision_tag & ECollisionTag::Y_AXIS) {
            m_speed.y = 0;
        }
    }

    switch (m_state) {
    case State::EGG:
        if (m_collision_tag & ECollisionTag::FLOOR) {
            setState(State::NORMAL);
        }
        break;
    case State::NORMAL:
        m_animator.flipX(m_speed.x > 0);
        break;
    case State::DIED:
        updatePhysics(delta_time, GRAVITY_FORCE / 2);
        break;
    }
}

void Spinny::takeDamage(DamageType damageType, Character* attacker) {
    if (damageType == DamageType::HIT_FROM_ABOVE) {
        attacker->takeDamage(DamageType::KICK, this);
    } else {
        setState(State::DIED);
    }
}

void Spinny::touch(Character* character) {
    character->takeDamage(DamageType::KICK, this);
}

bool Spinny::isAlive() const {
    return (m_state != State::DIED);
}
//---------------------------------------------------------------------------
// ! Lakity
//---------------------------------------------------------------------------
Lakity::Lakity() {
    setName("Lakity");
    setSize({ 32, 48 });
    const auto& texture = *MARIO_GAME.textureManager().get("Enemies");
    m_animator.create("fire", texture, { 0, 128, 32, 48 });
    m_animator.create("fly",  texture, { 32, 128, 32, 48 });
    m_animator.create("died", texture, { 32, 128 + 48, 32, -48 });
    setState(State::NORMAL);
}

void Lakity::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

void Lakity::update(int delta_time) {
    Enemy::update(delta_time);

    switch (m_state) {
    case State::NORMAL: {
        // move porcessing
        float diff_x = mario()->getPosition().x - getPosition().x;
        m_speed.x += math::sign(diff_x)*sqrt(std::abs(diff_x)) / 4000;
        m_speed.x = math::clamp(m_speed.x, -0.35f, 0.35f);

        move(m_speed*delta_time);

        // fire processing
        m_fire_timer += delta_time;
        if (m_fire_timer > FIRE_RATE) {
            Vector fly_direction = isCharacterInFront(mario(), this) ? Vector::RIGHT
                                                                     : Vector::LEFT;
            Vector spinny_position = getPosition() + Vector(0, 10);
            Vector spinny_speed = Vector(0.05f * fly_direction.x, 0.2f);
            MARIO_GAME.spawnObject<Spinny>(spinny_position, spinny_speed, fly_direction);

            m_fire_timer = 0;
            playAnimation("fly");
        }
        if (m_fire_timer > FIRE_RATE * 0.8f) {
            playAnimation("fire");
        }
        break;
    }
    case State::RUN_AWAY:
        move(m_speed * delta_time);
        m_died_timer += delta_time;
        if (m_died_timer > 2000) {
            removeLater();
        }
        break;
    case State::DIED:
        updatePhysics(delta_time, GRAVITY_FORCE / 2);
        break;
    }
}

void Lakity::takeDamage(DamageType damageType, Character* attacker) {
    setState(State::DIED);
}

void Lakity::touch(Character* character) {
    character->takeDamage(DamageType::KICK, character);
}

bool Lakity::isAlive() const {
    return (m_state != State::DIED);
}

void Lakity::setState(State state) {
    m_state = state;
    if (m_state == State::DIED) {
        m_animator.play("died");
        m_speed = Vector::ZERO;
        addScoreToPlayer(1200);
        MARIO_GAME.playSound("kick");
    } else if (m_state == State::NORMAL) {
        m_animator.play("fly");
    }
}

void Lakity::runAway(const Vector& run_direction) {
    m_speed.x = run_direction.x * 0.2f;
    setState(State::RUN_AWAY);
}

CheepCheep::CheepCheep(const Vector& initial_pos, const Vector& initial_speed) {
    const sf::Texture& texture = *MARIO_GAME.textureManager().get("Enemies");
    m_animator.create("fly", texture, Vector(0, 176), Vector(32, 32), 2, 1, 0.005f);
    m_animator.create("died", texture, { 0, 176 + 32, 32, -32 });
    setSize({ 32, 32 });

    m_speed = initial_speed;
    setPosition(initial_pos);
    setState(State::NORMAL);
}

CheepCheep::CheepCheep() {
    const sf::Texture& texture = *MARIO_GAME.textureManager().get("Enemies");
    m_animator.create("fly", texture, Vector(0, 176), Vector(32, 32), 2, 1, 0.005f);
    m_animator.create("died", texture, { 0, 176 + 32, 32, -32 });
    setSize({ 32, 32 });

    m_speed = Vector::LEFT * 0.05f;
    setState(State::UNDERWATER);
}

void CheepCheep::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

void CheepCheep::update(int delta_time) {
    Enemy::update(delta_time);
    switch (m_state) {
    case State::NORMAL:
        updatePhysics(delta_time, GRAVITY_FORCE*0.4f);
        m_animator.update(delta_time);
        break;
    case State::UNDERWATER:
        if (std::abs(mario()->getPosition().x - getPosition().x) < MARIO_GAME.screenSize().x / 2) {
            move(m_speed * delta_time);
            m_animator.update(delta_time);
        }
        break;
    case State::DIED:
        updatePhysics(delta_time, GRAVITY_FORCE);
        break;
    }
}

void CheepCheep::takeDamage(DamageType damageType, Character* attacker) {
    setState(State::DIED);
}

void CheepCheep::touch(Character* character) {
    character->takeDamage(DamageType::KICK, character);
}

bool CheepCheep::isAlive() const {
    return (m_state != State::DIED);
}

void CheepCheep::setState(State state) {
    m_state = state;

    if (m_state == State::NORMAL) {
        playAnimation("fly");
        m_animator.flipX(m_speed.x > 0);
    } else if (m_state == State::DIED) {
        m_speed = Vector::ZERO;
        addScoreToPlayer(200);
        playAnimation("died");
        MARIO_GAME.playSound("kick");
    }
}
//---------------------------------------------------------------------------
//! Blooper
//---------------------------------------------------------------------------
Blooper::Blooper() {
    setSize({ 32, 48 });
    const sf::Texture& texture = *MARIO_GAME.textureManager().get("Enemies");
    m_animator.create("zig", texture, { 224,161,32,48 });
    m_animator.create("zag", texture, { 256,161,32,48 });
    m_animator.create("died", texture, { 224,161 + 48,32,-48 });
}

void Blooper::enterState(State state) {
    m_state = state;
    switch (state) {
    case Blooper::ZIG:
        m_speed = -Vector(1, 1) * 0.15f;
        m_animator.play("zig");
        m_delay_time = 400;
        break;
    case Blooper::ZAG:
        m_speed = Vector::DOWN * 0.05f;
        m_animator.play("zag");
        m_delay_time = 1200;
        break;
    case Blooper::DIED:
        m_animator.play("died");
        m_speed = Vector::DOWN * 0.2f;
        break;
    }
}

void Blooper::update(int delta_time) {
    Enemy::update(delta_time);

    if (std::abs(mario()->getPosition().x - getPosition().x) < MARIO_GAME.screenSize().x / 2) {
        switch (m_state) {
        case Blooper::ZIG:
            m_delay_time -= delta_time;
            if (m_delay_time < 0)
                enterState(State::ZAG);
            break;
        case Blooper::ZAG:
            m_delay_time -= delta_time;
            if (m_delay_time < 0)
                enterState(State::ZIG);
            break;
        }
        move(delta_time*m_speed);
        m_animator.update(delta_time);
    }
}

void Blooper::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

void Blooper::takeDamage(DamageType damageType, Character* attacker) {
    if (damageType == DamageType::HIT_FROM_ABOVE) {
        attacker->takeDamage(DamageType::KICK, this);
    } else {
        enterState(State::DIED);
    }
}

void Blooper::touch(Character* character) {
    character->takeDamage(DamageType::KICK, character);
}

bool Blooper::isAlive() const {
    return (m_state != State::DIED);
}
//---------------------------------------------------------------------------
//! BulletBill
//---------------------------------------------------------------------------
BulletBill::BulletBill(const Vector& initial_pos, const Vector& initial_speed) {
    setSize({ 32, 32 });
    m_speed = initial_speed;
    setPosition(initial_pos);
    const sf::Texture& texture = *MARIO_GAME.textureManager().get("Enemies");
    m_animator.create("fly", texture, Vector(64, 112), Vector(32, 32), 1, 3, 0.005f);
    m_animator.create("died", texture, { 64,176 + 32,32,-32 });
    setState(State::NORMAL);
}

void BulletBill::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

void BulletBill::update(int delta_time) {
    Enemy::update(delta_time);

    switch (m_state) {
    case State::NORMAL: {
        updatePhysics(delta_time, 0);
        m_animator.update(delta_time);
        int camera_width = getParent()->castTo<MarioGameScene>()->cameraRect().width();
        int distance_to_mario = std::abs(getPosition().x - mario()->getPosition().x);
        if (distance_to_mario > camera_width) {
            removeLater();
        }
        break;
    }
    case State::DIED:
        updatePhysics(delta_time, GRAVITY_FORCE);
        break;
    }
}

void BulletBill::takeDamage(DamageType damageType, Character* attacker) {
    setState(State::DIED);
}

void BulletBill::touch(Character* character) {
    character->takeDamage(DamageType::KICK, character);
}

bool BulletBill::isAlive() const {
    return (m_state != State::DIED);
}

void BulletBill::setState(State state) {
    m_state = state;
    if (m_state == State::NORMAL)
    {
        playAnimation("fly");
        m_animator.flipX(m_speed.x > 0);
    }
    else if (m_state == State::DIED)
    {
        m_speed = Vector::ZERO;
        m_animator.play("died");
        addScoreToPlayer(1000);
        MARIO_GAME.playSound("kick");
    }
}
//---------------------------------------------------------------------------
//! PiranhaPlant
//---------------------------------------------------------------------------
PiranhaPlant::PiranhaPlant() {
    auto texture = MARIO_GAME.textureManager().get("Enemies");
    m_animator.create("open",   *texture, Vector(32, 80), SIZE, 1, 1, 1);
    m_animator.create("close",  *texture, Vector(0,  80), SIZE, 1, 1, 1);
    m_animator.create("biting", *texture, Vector(0, 80), SIZE, 2, 1, 0.01);
}

void PiranhaPlant::takeDamage(DamageType damageType, Character* attacker) {
    if (damageType == DamageType::HIT_FROM_ABOVE) {
        if (m_dead_zone) {
            attacker->takeDamage(DamageType::KICK, this);
        }
    } else {
        removeLater();
        addScoreToPlayer(800);
        MARIO_GAME.playSound("kick");
    }
}

bool PiranhaPlant::isAlive() const {
    return true;
}

void PiranhaPlant::touch(Character* character) {
    if (m_dead_zone) {
        character->takeDamage(DamageType::KICK, character);
    }
}

void PiranhaPlant::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

void PiranhaPlant::update(int delta_time) {
    Enemy::update(delta_time);
    m_animator.update(delta_time);

    m_timer += delta_time;

    m_dead_zone = ((m_timer > 0.25 * PERIOD_MS) &&
                   (m_timer < 3 * PERIOD_MS));

    float height = 0;

    if (m_timer < PERIOD_MS) { // none
        height = 0;
        // too close to mario - don't appear
        if ((mario()->getBounds().center() - getBounds().center()).length() < 100) {
            hideInTube();
        }
    } else if (m_timer < 1.25 * PERIOD_MS) { // appearing
        playAnimation("open");
        height = ((m_timer - PERIOD_MS) / (0.25f * PERIOD_MS)) * SIZE.y;
    } else if (m_timer < 3 * PERIOD_MS) {    // in full size
        playAnimation("biting");
        height = SIZE.y;
    } else if (m_timer < 3.25 * PERIOD_MS) { // hiding
        playAnimation("close");
        height = (1 - ((m_timer - 3 * PERIOD_MS) / (0.25f * PERIOD_MS))) * SIZE.y;
    } else {
        m_timer = 0;
    }

    setSize({ SIZE.x, height });
    move({0.f, m_buttom - getPosition().y - height });
}

void PiranhaPlant::hideInTube() {
    m_timer = 0;
    setSize({ SIZE.x, 0.f });
    move({ 0.f, m_buttom - getPosition().y });
}

void PiranhaPlant::onStarted() {
    Enemy::onStarted();
    m_buttom = getPosition().y + SIZE.y;
    // moveToBack();
}
//--------------------------------------------------------------------------
//! Podoboo
//--------------------------------------------------------------------------
Podoboo::Podoboo() {
    setSize({ 32,32 });
    const sf::Texture& texture = *MARIO_GAME.textureManager().get("Enemies");
    m_animator.create("up", texture, Vector(192, 80), Vector(32, 32), 3, 1, 0.005f);
    m_animator.create("down", texture, Vector(192, 112), Vector(32, -32), 3, 1, 0.005f);
}

void Podoboo::takeDamage(DamageType damageType, Character* attacker) {
    if (damageType == DamageType::HIT_FROM_ABOVE) {
        attacker->takeDamage(DamageType::SHOOT, this);
    }
}

bool Podoboo::isAlive() const {
    return true;
}

void Podoboo::touch(Character* character) {
    character->takeDamage(DamageType::SHOOT, character);
}

void Podoboo::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

void Podoboo::update(int delta_time) {
    Enemy::update(delta_time);
    m_timer += delta_time;

    if (getPosition().y > m_center.y) {
        m_speed += Vector::UP * m_acceleration * delta_time;
    } else {
        m_speed += Vector::DOWN * m_acceleration * delta_time;
    }

    if (m_timer > PERIOD_TIME) { // synchronization
        setPosition(m_center);
        m_speed = Vector::UP * m_acceleration * PERIOD_TIME * 0.25;
        m_timer = 0;
    }

    m_animator.update(delta_time);
    move(m_speed * delta_time);
    m_animator.play((m_speed.y < 0) ? "down" : "up");
}

void Podoboo::onStarted() {
    Enemy::onStarted();
    m_center = getPosition();
    m_acceleration = AMPLITUDE / (PERIOD_TIME * PERIOD_TIME * 0.25f * 0.25f);
    m_speed = Vector::UP*m_acceleration * PERIOD_TIME * 0.25f;
}
//--------------------------------------------------------------------------
//! LakitySpawner
//--------------------------------------------------------------------------
void LakitySpawner::update(int delta_time) {
    GameObject::update(delta_time);

    m_lakity_checker_timer += delta_time;
    if (m_lakity_checker_timer > CHECK_INTERVAL) {
        m_lakity = getParent()->findChildObjectByType<Lakity>();
        Rect camera_rect = getParent()->castTo<MarioGameScene>()->cameraRect();

        if (!m_lakity) {
            // Add Lakity to the scene processing
            if (getBounds().isContainByX(m_mario->getPosition())) {
                m_lakity = MARIO_GAME.spawnObject<Lakity>();
                m_lakity->setPosition(camera_rect.left() - 32, 64);
            }
        }
        m_lakity_checker_timer = 0;
    }

    if (m_lakity) {
        // throw Lakity from the scene processing
        Rect camera_rect = getParent()->castTo<MarioGameScene>()->cameraRect();
        if (m_lakity->getPosition().x > getBounds().right() + camera_rect.size().x / 2) {
            m_lakity->runAway(Vector::LEFT);
            m_lakity = nullptr;
        } else if (m_lakity->getPosition().x < getBounds().left() - camera_rect.size().x / 2) {
            m_lakity->runAway(Vector::RIGHT);
            m_lakity = nullptr;
        }
    }
}

void LakitySpawner::onStarted() {
    setSize({ getProperty("width").asFloat(), getProperty("height").asFloat() });
    m_mario = MARIO_GAME.getPlayer();
}
//--------------------------------------------------------------------------
//! CheepCheepSpawner
//--------------------------------------------------------------------------
void CheepCheepSpawner::update(int delta_time) {
    m_spawn_timer += delta_time;

    if ((m_spawn_timer > SPAWN_INTERVAL) && getBounds().isContainByX(m_mario->getPosition())) {
        int camera_width = getParent()->castTo<MarioGameScene>()->cameraRect().width();
        int x = rand() % int(camera_width) - camera_width / 2 + m_mario->getPosition().x;
        Vector direction = (m_mario->getPosition().x < x) ? Vector::LEFT
                                                          : Vector::RIGHT;

        MARIO_GAME.spawnObject<CheepCheep>(Vector(x, m_map_height + 32), Vector(direction.x * spawn_speed.x, spawn_speed.y));
        m_spawn_timer = 0;
    }
}

void CheepCheepSpawner::onStarted() {
    setSize({ getProperty("width").asFloat(), getProperty("height").asFloat() });
    m_map_height = getParent()->findChildObjectByType<Blocks>()->getRenderBounds().height();
    m_mario = MARIO_GAME.getPlayer();
}
//--------------------------------------------------------------------------
//! BulletBillSpawner
//--------------------------------------------------------------------------
BulletBillSpawner::BulletBillSpawner() {
    m_spawn_timer = rand() % SPAWN_INTERVAL;
}

void BulletBillSpawner::update(int delta_time) {
    int camera_width = getParent()->castTo<MarioGameScene>()->cameraRect().width();
    m_spawn_timer += delta_time;
    bool is_mario_close_enough = std::abs(m_mario->getPosition().x - getPosition().x) < camera_width;
    bool is_time_to_push = m_spawn_timer > SPAWN_INTERVAL;
    bool is_bullet_bill_beyond_tiled_map = isBulletBillBeyondTiledMap();

    if (is_time_to_push && (is_mario_close_enough || is_bullet_bill_beyond_tiled_map)) {
        Vector direction = isCharacterInFront(m_mario, this) ? Vector::RIGHT
                                                             : Vector::LEFT;
        Vector pos = getPosition();

        if (is_bullet_bill_beyond_tiled_map) {
            int k = (pos.x < m_mario->getPosition().x ? -1 : 1);
            pos.x = m_mario->getPosition().x + k * MARIO_GAME.screenSize().x / 2;
        }

        getParent()->addChild(new BulletBill(pos, direction * BULLET_SPEED));
        m_spawn_timer = 0;
        if (is_bullet_bill_beyond_tiled_map) {
            m_spawn_timer = -4000 - rand() % 8000;
        }
        MARIO_GAME.playSound("fireworks");
	}
}

bool BulletBillSpawner::isBulletBillBeyondTiledMap() const {
	return (getPosition().x < 0) || (getPosition().x > m_blocks_width);
}

void BulletBillSpawner::onStarted() {
    m_mario = MARIO_GAME.getPlayer();
    m_blocks_width = getParent()->findChildObjectByType<Blocks>()->getRenderBounds().width();
    if (isBulletBillBeyondTiledMap()) {
       m_spawn_timer = - rand() % 5000;
    }
}
//--------------------------------------------------------------------------
//! Fireball
//--------------------------------------------------------------------------
Fireball::Fireball(const Vector& Position, const Vector& SpeedVector) {
    auto texture = MARIO_GAME.textureManager().get("Bowser");
    m_animator.create("fire", *texture, { 0,364 }, { 32,36 }, 4, 1, 0.01f, AnimType::FORWARD_BACKWARD_CYCLE);
    m_speed = SpeedVector;
    setPosition(Position);
    m_animator.flipX(SpeedVector.x < 0);
    m_animator.get("fire")->setOrigin({ 16,18 });
}

void Fireball::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

void Fireball::update(int delta_time) {
    m_animator.update(delta_time);
    m_life_timer -= delta_time;

    if (m_life_timer < 0) {
        removeLater();
    }

    move(delta_time * m_speed);

    if (m_mario->getBounds().isContain(getPosition())) {
        m_mario->takeDamage(DamageType::SHOOT, nullptr);
    }
}

void Fireball::onStarted() {
	m_mario = MARIO_GAME.getPlayer();
}
//--------------------------------------------------------------------------
//! Bowser
//--------------------------------------------------------------------------
Bowser::Bowser() {
    setSize({ 84,80 });
    auto texture = MARIO_GAME.textureManager().get("Bowser");
    m_animator.create("walk", *texture, { 0,0 }, { 84,80 }, 6, 1, ANIM_SPEED, AnimType::FORWARD_CYCLE);
    m_animator.create("died", *texture, { 0,80,84,-80 });
    m_animator.create("turn", *texture, { 381,122 }, { 74,85 }, 2, 1, ANIM_SPEED / 2, AnimType::FORWARD_STOP);
    m_animator.create("middle_fire", *texture, { 0,167 }, { 91,100 }, 4, 1, ANIM_SPEED, AnimType::FORWARD_STOP);
    m_animator.create("land_fire", *texture, { 0,267 }, { 92,97 }, 6, 1, ANIM_SPEED, AnimType::FORWARD_STOP);
    m_animator.create("pre_jump", *texture, { 0,80 }, { 91,79 }, 2, 1, ANIM_SPEED, AnimType::FORWARD_STOP);
    m_animator.create("up_jump", *texture, { 182,80,84,87 });
    m_animator.create("down_jump", *texture, { 266,80, 84,87 });
    m_animator.get("middle_fire")->setOrigin(Vector::DOWN * 16);
    m_animator.get("land_fire")->setOrigin(Vector::DOWN * 16);
    m_animator.get("turn")->setOrigin(Vector::DOWN * 5);
}

void Bowser::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

void Bowser::noBridge() {
    enterState(State::NO_BRIDGE);
}

void Bowser::enterState(State state) {
    if (state == m_state) {
        return;
    }

    LOG("AI", VERBOSE, "Bowser state changed to %d", m_state);

    m_state = state;

    switch (state) {
    case State::WALK:
        playAnimation("walk");
        m_delay_timer = 2000;
        break;
    case State::TURN:
        playAnimation("turn");
        m_delay_timer = 400;
        break;
    case State::PRE_JUMP:
        playAnimation("pre_jump");
        m_delay_timer = 300;
        break;
    case State::JUMP:
        playAnimation("up_jump");
        m_speed.y = -0.4f;
        m_old_speed.x = m_speed.x;
        m_speed.x = 0;
        break;
    case State::MIDDLE_FIRE:
        playAnimation("middle_fire");
        m_delay_timer = 500;
        break;
    case State::LAND_FIRE:
        playAnimation("land_fire");
        m_delay_timer = 700;
        break;
    case State::NO_BRIDGE:
        playAnimation("walk");
        m_animator.get("walk")->setSpeed(ANIM_SPEED * 2.5f);
        m_delay_timer = 1000;
        break;
    case State::FALL:
        m_animator.flipX(m_direction == Vector::RIGHT);
        m_animator.get("walk")->setSpeed(0);
        MARIO_GAME.playSound("bowser_falls");
        m_speed = Vector::ZERO;
        break;
    case(State::DIED):
        playAnimation("died");
        MARIO_GAME.playSound("bowser_falls");
        m_speed = Vector::ZERO;
        break;
    }
}

void Bowser::update(int delta_time) {
    auto processTimer = [this](int delta_time) -> bool {
            m_delay_timer -= delta_time;
            if (m_delay_timer < 0) {
                m_delay_timer = 0;
                return true;
            }
            return false;
        };

    Enemy::update(delta_time);
    m_animator.update(delta_time);

    if ((m_state == State::WALK) ||
        (m_state == State::JUMP)) {
        updatePhysics(delta_time, GRAVITY_FORCE / 2);
        updateCollision(delta_time);
        if (m_collision_tag & ECollisionTag::X_AXIS) {
            m_speed.x = -m_speed.x;
        }
        if (m_collision_tag & ECollisionTag::Y_AXIS) {
            m_speed.y = 0.f;
        }
    }

    switch (m_state) {
    case State::WALK:
    {
        // Walk processing
        if (std::abs(getPosition().x - m_center_x) > WALK_AMPLITUDE) {
            m_speed.x = -m_speed.x;
        }

        auto old_direction = m_direction;
        m_direction = isCharacterInFront(mario(), this) ? Vector::RIGHT
                                                        : Vector::LEFT;

        if (old_direction != m_direction) {
            enterState(State::TURN);
            return;
        }

        m_animator.flipX(m_direction == Vector::RIGHT);

        if (processTimer(delta_time)) {
            int d = rand() % 3;
            if (d == 0) enterState(State::PRE_JUMP);
            if (d == 1) enterState(State::MIDDLE_FIRE);
            if (d == 2) enterState(State::LAND_FIRE);
        }
        break;
    }
    case State::TURN:
        if (processTimer(delta_time)) {
            enterState(State::WALK);
        }
        break;
    case State::JUMP:
        if ((m_speed.y >= 0) && (m_old_speed.y <= 0)) {
            // jump peak
            fire(Vector::DOWN * 20);
            playAnimation("down_jump");
        }
 
        m_old_speed.y = m_speed.y;
        if (m_collision_tag & ECollisionTag::FLOOR) {
            m_speed.x = m_old_speed.x;
            enterState(State::WALK);
        }
        break;
    case State::PRE_JUMP:
        if (processTimer(delta_time)) {
            enterState(State::JUMP);
        }
        break;
    case State::LAND_FIRE:
        if (processTimer(delta_time)) {
            fire(Vector::DOWN * 10);
            enterState(State::WALK);
        }
        break;
    case State::MIDDLE_FIRE:
        if (processTimer(delta_time)) {
            fire(Vector::UP * 10);
            enterState(State::WALK);
        }
        break;
    case State::NO_BRIDGE:
        if (processTimer(delta_time)) {
            enterState(State::FALL);
        }
        break;
    case State::DIED: // fall through
    case State::FALL:
        updatePhysics(delta_time, GRAVITY_FORCE / 2);
        break;
    }
}

void Bowser::fire(const Vector& fireBallOffset) {
    MARIO_GAME.spawnObject<Fireball>(getBounds().center() + m_direction * 50 +
                                     fireBallOffset, m_direction * 0.13f);
    MARIO_GAME.playSound("bowser_fire");
}

void Bowser::onStarted() {
    Enemy::onStarted();
    m_center_x = getPosition().x;
    m_speed.x = RUN_SPEED;
}

void Bowser::takeDamage(DamageType damageType, Character* attacker) {
    if (damageType == DamageType::HIT_FROM_ABOVE) {
        attacker->takeDamage(DamageType::KICK, this);
    } else {
        --m_lives;
        if (m_lives < 0) {
            enterState(State::DIED);
        }
    }
}

void Bowser::touch(Character* character) {
    character->takeDamage(DamageType::KICK, character);
}

bool Bowser::isAlive() const {
    return ((m_state != State::DIED) &&
            (m_state != State::FALL));
}
