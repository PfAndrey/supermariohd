#include "SuperMarioGame.hpp"
#include "Blooper.hpp"

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
        move(delta_time * m_speed);
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
    }
    else {
        enterState(State::DIED);
    }
}

void Blooper::touch(Character* character) {
    character->takeDamage(DamageType::KICK, character);
}

bool Blooper::isAlive() const {
    return (m_state != State::DIED);
}
