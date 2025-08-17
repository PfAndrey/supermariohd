#include "Blocks.hpp"
#include "CheepCheep.hpp"
#include "SuperMarioGame.hpp"

CheepCheep::CheepCheep(const Vector& initial_pos, const Vector& initial_speed) {
    const sf::Texture& texture = *MARIO_GAME.textureManager().get("Enemies");
    m_animator.create("fly", texture, Vector(0, 176), Vector(32, 32), 2, 1, 0.005f);
    m_animator.create("died", texture, { 0, 176 + 32, 32, -32 });
    setSize({ 32, 32 });

    m_velocity = initial_speed;
    setPosition(initial_pos);
    setState(State::NORMAL);
}

CheepCheep::CheepCheep() {
    const sf::Texture& texture = *MARIO_GAME.textureManager().get("Enemies");
    m_animator.create("fly", texture, Vector(0, 176), Vector(32, 32), 2, 1, 0.005f);
    m_animator.create("died", texture, { 0, 176 + 32, 32, -32 });
    setSize({ 32, 32 });

    m_velocity = Vector::LEFT * 0.05f;
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
        updatePhysics(delta_time, GRAVITY_FORCE * 0.4f);
        m_animator.update(delta_time);
        break;
    case State::UNDERWATER:
        if (std::abs(mario()->getPosition().x - getPosition().x) < MARIO_GAME.screenSize().x / 2) {
            move(m_velocity * delta_time);
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
        m_animator.flipX(m_velocity.x > 0);
    }
    else if (m_state == State::DIED) {
        m_velocity = Vector::ZERO;
        addScoreToPlayer(200);
        playAnimation("died");
        MARIO_GAME.playSound("kick");
    }
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
