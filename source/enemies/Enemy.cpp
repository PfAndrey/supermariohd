#include <cmath>

#include <Logger.hpp>
#include "Blocks.hpp"
#include "Enemies.hpp"
#include "Mario.hpp"
#include "SuperMarioGame.hpp"

void Enemy::checkNextTileUnderFoots() {
    if (m_velocity.y != 0) {
        return;
    }

    Vector own_center = getBounds().center();
    Vector opposite_vector = math::sign(m_velocity.x) * Vector::RIGHT;
    bool is_next_under_foot = m_blocks->isCollidableBlock(m_blocks->toBlockCoordinates(own_center + 20*opposite_vector + 32*Vector::DOWN));
    bool is_prev_under_foot = m_blocks->isCollidableBlock(m_blocks->toBlockCoordinates(own_center - 60*opposite_vector + 32*Vector::DOWN));
    bool is_prev_back = m_blocks->isCollidableBlock(m_blocks->toBlockCoordinates(own_center - 50*opposite_vector));
    bool is_next_back = m_blocks->isCollidableBlock(m_blocks->toBlockCoordinates(own_center + 50*opposite_vector));

    if ((!is_next_under_foot && !is_prev_back) && (is_next_under_foot || is_prev_under_foot)) {
        m_velocity.x = -m_velocity.x;
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

void Enemy::updateCollision(float delta_time, LogicFlags logicFlags) {
    m_collision_tag = ECollisionTag::NONE;
    setPosition(m_blocks->collsionResponse(getBounds(), m_velocity, delta_time, m_collision_tag));

    if (logicFlags & LogicFlags::ON_X_BOUND) {
        if (m_collision_tag & ECollisionTag::X_AXIS) {
            m_velocity.x = -m_velocity.x;
        }
    }

    if (m_collision_tag & ECollisionTag::Y_AXIS) {
        m_velocity.y = 0;
    }
}

void Enemy::updatePhysics(float delta_time, float gravity) {
    m_velocity += Vector::DOWN * gravity * delta_time;
    move(delta_time * m_velocity);
}

bool Enemy::isInCamera() const {
    Rect cameraRect = getParent()->castTo<MarioGameScene>()->cameraRect();
    return (std::abs(getPosition().x - cameraRect.center().x) < cameraRect.width() / 2);
}

void Enemy::playAnimation(const std::string& name) {
    m_animator.play(name);
}

void Enemy::playSound(const std::string& name) {
    MARIO_GAME.playSound(name);
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

bool Enemy::isCharacterInFront(Character* target, GameObject* origin) {
    return (target->getBounds().center().x > origin->getBounds().center().x);
}
