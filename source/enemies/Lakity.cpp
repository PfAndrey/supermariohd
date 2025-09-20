#include <cmath>

#include "SuperMarioGame.hpp"
#include "Lakity.hpp"

Spinny::Spinny(const Vector& position, const Vector& speed, const Vector& walk_direction) {
    setPosition(position);
    m_velocity = speed;
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

    switch (m_state) {
    case State::NORMAL:
        playAnimation("walk");
        m_velocity.x = RUN_SPEED;
        if (m_walk_direction == Vector::RIGHT) {
            m_velocity = -m_velocity;
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
        updateCollision(delta_time, LogicFlags::ON_X_BOUND);
    }

    switch (m_state) {
    case State::EGG:
        if (m_collision_tag & ECollisionTag::FLOOR) {
            setState(State::NORMAL);
        }
        break;
    case State::NORMAL:
        m_animator.flipX(m_velocity.x > 0);
        break;
    case State::DIED:
        updatePhysics(delta_time, GRAVITY_FORCE / 2);
        break;
    }
}

void Spinny::takeDamage(DamageType damageType, Character* attacker) {
    if (damageType == DamageType::HIT_FROM_ABOVE) {
        attacker->takeDamage(DamageType::KICK, this);
    }
    else {
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
    m_animator.create("fly", texture, { 32, 128, 32, 48 });
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
        m_velocity.x += math::sign(diff_x) * sqrt(std::abs(diff_x)) / 4000;
        m_velocity.x = math::clamp(m_velocity.x, -0.35f, 0.35f);

        move(m_velocity * delta_time);

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
        move(m_velocity * delta_time);
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
        m_velocity = Vector::ZERO;
        addScoreToPlayer(1200);
        MARIO_GAME.playSound("kick");
    }
    else if (m_state == State::NORMAL) {
        m_animator.play("fly");
    }
}

void Lakity::runAway(const Vector& run_direction) {
    m_velocity.x = run_direction.x * 0.2f;
    setState(State::RUN_AWAY);
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
        }
        else if (m_lakity->getPosition().x < getBounds().left() - camera_rect.size().x / 2) {
            m_lakity->runAway(Vector::RIGHT);
            m_lakity = nullptr;
        }
    }
}

void LakitySpawner::onStarted() {
    setSize({ getProperty("width").asFloat(), getProperty("height").asFloat() });
    m_mario = MARIO_GAME.getPlayer();
}
