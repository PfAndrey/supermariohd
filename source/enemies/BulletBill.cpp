#include "Blocks.hpp"
#include "SuperMarioGame.hpp"
#include "BulletBill.hpp"

BulletBill::BulletBill(const Vector& initial_pos, const Vector& initial_speed) {
    setSize({ 32, 32 });
    m_velocity = initial_speed;
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
        m_animator.flipX(m_velocity.x > 0);
    }
    else if (m_state == State::DIED)
    {
        m_velocity = Vector::ZERO;
        m_animator.play("died");
        addScoreToPlayer(1000);
        MARIO_GAME.playSound("kick");
    }
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
        Vector direction = Enemy::isCharacterInFront(m_mario, this) ? Vector::RIGHT
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
        m_spawn_timer = -rand() % 5000;
    }
}