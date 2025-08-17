#include "Blocks.hpp"
#include "SuperMarioGame.hpp"
#include "HammerBro.hpp"

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
        move(m_speed * delta_time);

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
    m_velocity.x = RUN_SPEED;
}

void HammerBro::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

void HammerBro::update(int delta_time) {
    Enemy::update(delta_time);

    if (m_velocity.y == 0) {
        m_animator.update(delta_time);
    }

    switch (m_state) {
    case State::NORMAL:
        // Physic processing
        updatePhysics(delta_time, GRAVITY_FORCE / 2);

        if (m_collision_on) {
            updateCollision(delta_time, LogicFlags::ON_X_BOUND);
        }

        // Walk processing
        if (std::abs(getPosition().x - m_center_x) > WALK_AMPLITUDE) {
            m_velocity.x = -m_velocity.x;
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
                m_velocity += Vector::UP * 0.5;
            }
            else {                                // jump-off down
                m_velocity += Vector::UP * 0.25;
                m_drop_off_height = getPosition().y + getBounds().height() + 32.f;
            }

            m_collision_on = false;                // turn off collision check for moving through walls
            m_jump_timer = 0;
        }

        if (!m_collision_on) {                     // turn on collision check for take ground
            if (m_jump_direction == Vector::UP) {
                m_collision_on = (m_velocity.y > 0);
            }
            else {
                m_collision_on = (getPosition().y > m_drop_off_height);
            }
        }

        // Fire processing
        m_fire_timer += delta_time;
        if (m_fire_timer < FIRE_RATE / 2) {
            // just walk
        }
        else if (m_fire_timer < FIRE_RATE) {
            // get hammer in hand
            if (!m_hummer) {
                m_hummer = MARIO_GAME.spawnObject<Hammer>(mario());
                playAnimation("walk_with_hammer");
            }
            const Vector hand_off_set = { -3 * m_direction.x, -22.f };
            m_hummer->setPosition(getPosition() + hand_off_set);
        }
        else {
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
        m_velocity.y = 0;
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
