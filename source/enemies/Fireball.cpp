#include "SuperMarioGame.hpp"
#include "Fireball.hpp"

Fireball::Fireball(const Vector& Position, const Vector& SpeedVector) {
    auto texture = MARIO_GAME.textureManager().get("Bowser");
    m_animator.create("fire", *texture, { 0,364 }, { 32,36 }, 4, 1, 0.01f, AnimType::FORWARD_BACKWARD_CYCLE);
    m_speed = SpeedVector;
    setPosition(Position);
    m_animator.flipX(SpeedVector.x < 0);
    m_animator.setOrigin("fire", { 16,18 });
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
