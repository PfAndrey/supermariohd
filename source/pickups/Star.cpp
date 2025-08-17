#include "Blocks.hpp"
#include "SuperMarioGame.hpp"
#include "Star.hpp"

Star::Star(const Vector& pos) {
    m_sprite.setTexture(*MARIO_GAME.textureManager().get("Items"));
    m_sprite.setTextureRect({ 0,0,0,0 });
    setSize({ 31, 1 });
    setPosition(pos.x, pos.y + 32);
    m_speed.y = -JUMP_POWER;
    m_speed.x = HORIZONTAL_SPEED;
}

void Star::draw(sf::RenderWindow* render_window) {
    m_sprite.setPosition(getPosition());
    render_window->draw(m_sprite);
}

void Star::onStarted() {
    m_blocks = getParent()->findChildObjectByType<Blocks>();
    m_mario = MARIO_GAME.getPlayer();
}

void Star::update(int delta_time) {
    m_timer += delta_time;

    int sprite_index = int(m_timer *0.01) % 5;
    if (sprite_index > 2) {
        sprite_index = 5 - sprite_index;
    }

    switch (m_state) {
    case State::WAIT:
        if (m_timer > 400) {
            m_state = State::BORNING;
            MARIO_GAME.playSound("powerup_appears");
            m_timer = 0;
        }
        break;
    case State::BORNING: {
        int height = m_timer * 0.02;
        if (height <= 32) {
            m_sprite.setTextureRect({ 128 + sprite_index * 32 ,212,32,height });
            auto old_bounds = getBounds();
            old_bounds.setTop(old_bounds.bottom() - height);
            setBounds(old_bounds);
        } else {
            m_state = State::NORMAL;
        }
        break;
    }
    case State::NORMAL: {
        m_sprite.setTextureRect({ 128 + sprite_index * 32 ,212,32,32 });

        //update collissions
        auto collision_tag = ECollisionTag::NONE;
        m_blocks->collsionResponse(getBounds(), m_speed, delta_time, collision_tag);

        //jumping processing
        if (collision_tag & ECollisionTag::FLOOR) {
            m_speed.y = -JUMP_POWER;
        }

        if (collision_tag & ECollisionTag::X_AXIS) {
            m_speed.x = -m_speed.x;
        }

        //update physics
        m_speed += Vector::DOWN * GRAVITY_FORCE * delta_time;
        move(delta_time * m_speed);

        //check if fall undergound
        if (getPosition().y > 1000) {
            removeLater();
        }

        //check if mario pickup star
        if (m_mario && m_mario->getBounds().isIntersect(getBounds())) {
            removeLater();
            MARIO_GAME.invincibleMode(true);
        }
        break;
    }
    }
}
