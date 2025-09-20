#include "SuperMarioGame.hpp"
#include "Blocks.hpp"
#include "Fireflower.hpp"

FireFlower::FireFlower(const Vector& pos) 
    : m_sprite(*MARIO_GAME.textureManager().get("Items"),
        {{ 32, 212 }, { 32, 0 }})
{
    setSize({ 31, 1 });
    setPosition(pos.x, pos.y + 32);
}

void FireFlower::draw(sf::RenderWindow* render_window) {
    m_sprite.setPosition(getPosition());
    render_window->draw(m_sprite);
}

void FireFlower::update(int delta_time) {
    m_timer += delta_time;

    switch (m_state)
    {
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
            m_sprite.setTextureRect({{m_sprite.getTextureRect().position.x, m_sprite.getTextureRect().position.y}, {32, height} });
            auto old_bounds = getBounds();
            old_bounds.setTop(old_bounds.bottom() - height);
            setBounds(old_bounds);
        } else {
            m_state = State::NORMAL;
            m_speed = Vector::ZERO;
        }
        break;
    }
    case State::NORMAL:
        int sprite_index = int(m_timer * 0.01) % 5;
        if (sprite_index > 2) {
            sprite_index = 5 - sprite_index;
        }
        m_sprite.setTextureRect({{32 + sprite_index * 32 ,212}, {32,32} });

        if (m_mario->getBounds().isIntersect(getBounds())) {
            action();
            removeLater();
        }
        break;
    }
}

void FireFlower::action() {
    m_mario->promote();
    MARIO_GAME.addScore(1000, getBounds().center());
    MARIO_GAME.playSound("powerup");
}

void FireFlower::onStarted() {
    m_blocks = getParent()->findChildObjectByType<Blocks>();
    m_mario = MARIO_GAME.getPlayer();
}
