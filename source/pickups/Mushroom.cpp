#include "Blocks.hpp"
#include "Pickups.hpp"
#include "SuperMarioGame.hpp"
#include "Mario.hpp"

Mushroom::Mushroom(const Vector& pos)
    : m_sprite(*MARIO_GAME.textureManager().get("Items"),
        {{128, 150}, {32, 0}})
 {
    setSize({ 31, 1 });
    setPosition(pos.x, pos.y + 32);
}

void Mushroom::draw(sf::RenderWindow* render_window) {
    m_sprite.setPosition(getPosition());
    render_window->draw(m_sprite);
}

void Mushroom::update(int delta_time) {
    m_timer += delta_time;
    if (getPosition().y > 1000) { // falled underground
        removeLater();
    }

    switch (m_state) {
    case State::WAIT:
        if (m_timer > 400) {
            MARIO_GAME.playSound("powerup_appears");
            m_timer = 0;
            m_state = State::GROVING;
        }
        break;
    case State::GROVING: {
            int height = m_timer * GROVING_RATE;
            if (height <= 32) {
                m_sprite.setTextureRect({ {m_sprite.getTextureRect().position.x, m_sprite.getTextureRect().position.y}, {32, height}});
                auto old_bounds = getBounds();
                old_bounds.setTop(old_bounds.bottom() - height);
                setBounds(old_bounds);
            } else {
                m_state = State::NORMAL;
                m_speed.x = RUN_SPEED;
            }
            break;
        }
    case State::NORMAL: {
            m_speed += Vector::DOWN * GRAVITY_FORCE * delta_time;   // Gravity force
            move(delta_time*m_speed);
            ECollisionTag collision_tag = ECollisionTag::NONE;
            setPosition(m_blocks->collsionResponse(getBounds(), m_speed, delta_time, collision_tag));
            if (collision_tag & ECollisionTag::X_AXIS) {
                m_speed.x = -m_speed.x;
            }
            if (collision_tag & ECollisionTag::Y_AXIS) {
                m_speed.y = 0;
            }
        }
        if (m_mario && m_mario->getBounds().isIntersect(getBounds())) {
            action();
            removeLater();
        }
        break;
    }
}

void Mushroom::action() {
    m_mario->promote();
    MARIO_GAME.addScore(1000, getBounds().center());
    MARIO_GAME.playSound("powerup");
}

void Mushroom::onStarted() {
    m_blocks = getParent()->findChildObjectByType<Blocks>();
    m_mario = MARIO_GAME.getPlayer();
}

void Mushroom::kick() {
    m_speed += Vector::UP * 0.4f;
}

OneUpMushroom::OneUpMushroom(const Vector& pos)
    : Mushroom(pos) {
    m_as_flower = false;
    m_sprite.setTextureRect({{160,150}, {32,0}});
}

void OneUpMushroom::action() {
    MARIO_GAME.addLive();
    MARIO_GAME.playSound("1-up");
}