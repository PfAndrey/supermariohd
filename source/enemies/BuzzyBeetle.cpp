#include "SuperMarioGame.hpp"
#include "BuzzyBeetle.hpp"

BuzzyBeetle::BuzzyBeetle() {
    setSize({ 32, 32 });
    const sf::Texture& texture = *MARIO_GAME.textureManager().get("Enemies");
    m_animator.create("walk", texture, { {{96,0}, {32,32}},{{128,0}, {32,32}} }, 0.005f);
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
            if (m_velocity.y == 0) {
                setState(State::HIDDEN);
                addScoreToPlayer(100);
                MARIO_GAME.playSound("stomp");
            }
        case State::HIDDEN:
            setState(State::BULLET);

            m_velocity.x = isCharacterInFront(attacker, this) ? -std::abs(RUN_SPEED) * 6
                : std::abs(RUN_SPEED) * 6;
            addScoreToPlayer(400);
            MARIO_GAME.playSound("kick");
        case State::BULLET:
            setState(State::HIDDEN);
            break;
        }
    }
    else if (damageType == DamageType::HIT_FROM_BELOW) {
        // Handle case when other (Koopa or BuzzyBeetle) hit this BuzzyBeetle from below (bullet state)
        if ((damageType == DamageType::HIT_FROM_BELOW) && attacker && attacker->isAlive() && attacker->isTypeOf<Enemy>()) {
            attacker->takeDamage(DamageType::HIT_FROM_BELOW, nullptr);
        }

        setState(State::DIED);
        m_velocity.y = -0.4f;
        addScoreToPlayer(800);
        MARIO_GAME.playSound("kick");
    }
}

void BuzzyBeetle::touch(Character* character) {
    switch (m_state)
    {
    case State::HIDDEN:
        setState(State::BULLET);
        m_velocity.x = isCharacterInFront(character, this) ? -std::abs(RUN_SPEED) * 6
            : std::abs(RUN_SPEED) * 6;
        move(14 * Vector::RIGHT * math::sign(m_velocity.x));
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
        m_velocity.x = RUN_SPEED;
        playAnimation("walk");
        break;
    case State::HIDDEN:
        m_velocity.x = 0;
        playAnimation("hidden");
        break;
    case State::BULLET:
        m_velocity.x = 6 * RUN_SPEED;
        playAnimation("bullet");
        break;
    case State::DIED:
        m_velocity.y = -0.4f;
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
        updateCollision(delta_time, LogicFlags::ON_X_BOUND);
        m_animator.update(delta_time);
        m_animator.flipX(m_velocity.x > 0);
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

    m_animator.flipX(m_velocity.x > 0);
    m_animator.update(delta_time);
}
