#include "Blocks.hpp"
#include "Pickups.hpp"
#include "SuperMarioGame.hpp"
#include "Mario.hpp"


Coin::Coin() {
    auto texture = MARIO_GAME.textureManager().get("Items");
    m_animator.create("twist", *texture, Vector(0, 84), Vector(32, 32), 4, 1, 0.01f);
    m_animator.create("shine", *texture, Vector(0, 116), Vector(40, 32), 5, 1, 0.01f, AnimType::FORWARD);
    m_animator.get("shine")->setOrigin(Vector(4, 0));
    static float rot_offset = 0;
    rot_offset += 0.4f;
}

void Coin::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

void Coin::update(int delta_time) {
    m_animator.update(delta_time);

    switch (m_state) {
    case State::TWIST:
        if (m_mario->getBounds().isIntersect(getBounds())) {
            MARIO_GAME.addScore(100);
            MARIO_GAME.addCoin();
            MARIO_GAME.playSound("coin");
            m_animator.play("shine");
            m_state = State::SHANE;
        }
        break;
    case State::SHANE:
        m_remove_timer -= delta_time;
        if (m_remove_timer < 0) {
            removeLater();
        }
        break;
    }
}

void Coin::kick() {
    MARIO_GAME.addScore(100);
    MARIO_GAME.addCoin();
    MARIO_GAME.playSound("coin");
    MARIO_GAME.spawnObject<TwistedCoin>(getPosition() + Vector::UP * 32);
    removeLater();
}

void Coin::onStarted() {
    setSize({ getProperty("height").asFloat(),getProperty("width").asFloat() });
    m_mario = MARIO_GAME.getPlayer();
}
//---------------------------------------------------------------------------
//! Mushroom
//---------------------------------------------------------------------------
Mushroom::Mushroom(const Vector& pos) {
    setSize({ 31, 1 });
    setPosition(pos.x, pos.y + 32);
    m_sprite.setTexture(*MARIO_GAME.textureManager().get("Items"));
    m_sprite.setTextureRect(sf::IntRect(128, 150, 32, 0));
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
                m_sprite.setTextureRect({ m_sprite.getTextureRect().left, m_sprite.getTextureRect().top,32, height });
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
//---------------------------------------------------------------------------
//! FireFlower
//---------------------------------------------------------------------------
FireFlower::FireFlower(const Vector& pos) {
    setSize({ 31, 1 });
    setPosition(pos.x, pos.y + 32);
    m_sprite.setTexture(*MARIO_GAME.textureManager().get("Items"));
    m_sprite.setTextureRect(sf::IntRect(32, 212, 32, 0));
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
            m_sprite.setTextureRect({ m_sprite.getTextureRect().left, m_sprite.getTextureRect().top,32, height });
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
        m_sprite.setTextureRect({ 32 + sprite_index * 32 ,212,32,32 });

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
//---------------------------------------------------------------------------
//! OneUpMushroom
//---------------------------------------------------------------------------
OneUpMushroom::OneUpMushroom(const Vector& pos) 
    : Mushroom(pos) {
    m_as_flower = false;
    m_sprite.setTextureRect({ 160,150,32,0 });
}

void OneUpMushroom::action() {
    MARIO_GAME.addLive();
    MARIO_GAME.playSound("1-up");
}
//---------------------------------------------------------------------------
//! Star
//---------------------------------------------------------------------------
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
