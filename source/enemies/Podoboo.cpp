#include "SuperMarioGame.hpp"
#include "Podoboo.hpp"

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

    Vector forceVector = (getPosition().y > m_center.y) ? Vector::UP
                                                        : Vector::DOWN;
    m_velocity += forceVector * m_acceleration * delta_time;
 

    if (m_timer > PERIOD_TIME) { // synchronization
        setPosition(m_center);
        m_velocity = Vector::UP * m_acceleration * PERIOD_TIME * 0.25;
        m_timer = 0;
    }

    m_animator.update(delta_time);
    move(m_velocity * delta_time);
    m_animator.play((m_velocity.y < 0) ? "down" : "up");
}

void Podoboo::onStarted() {
    Enemy::onStarted();
    m_center = getPosition();
    m_acceleration = AMPLITUDE / (PERIOD_TIME * PERIOD_TIME * 0.25f * 0.25f);
    m_velocity = Vector::UP * m_acceleration * PERIOD_TIME * 0.25f;
}
