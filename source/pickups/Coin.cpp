#include "Blocks.hpp"
#include "SuperMarioGame.hpp"
#include "Coin.hpp"

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
