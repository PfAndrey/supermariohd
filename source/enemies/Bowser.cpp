#include "SuperMarioGame.hpp"
#include "Fireball.hpp"
#include "Bowser.hpp"

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
    m_animator.setOrigin("middle_fire", Vector::DOWN * 16);
    m_animator.setOrigin("land_fire", Vector::DOWN * 16);
    m_animator.setOrigin("turn", Vector::DOWN * 5);
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
        m_velocity.y = -0.4f;
        m_old_speed.x = m_velocity.x;
        m_velocity.x = 0;
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
        m_animator.setSpeed("walk", ANIM_SPEED * 2.5f);
        m_delay_timer = 1000;
        break;
    case State::FALL:
        m_animator.flipX(m_direction == Vector::RIGHT);
        m_animator.setSpeed("walk", 0);
        MARIO_GAME.playSound("bowser_falls");
        m_velocity = Vector::ZERO;
        break;
    case(State::DIED):
        playAnimation("died");
        MARIO_GAME.playSound("bowser_falls");
        m_velocity = Vector::ZERO;
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
        updateCollision(delta_time, LogicFlags::ON_X_BOUND);
    }

    switch (m_state) {
    case State::WALK:
    {
        // Walk processing
        if (std::abs(getPosition().x - m_center_x) > WALK_AMPLITUDE) {
            m_velocity.x = -m_velocity.x;
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
        if ((m_velocity.y >= 0) && (m_old_speed.y <= 0)) {
            // jump peak
            fire(Vector::DOWN * 20);
            playAnimation("down_jump");
        }

        m_old_speed.y = m_velocity.y;
        if (m_collision_tag & ECollisionTag::FLOOR) {
            m_velocity.x = m_old_speed.x;
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
    m_velocity.x = RUN_SPEED;
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
