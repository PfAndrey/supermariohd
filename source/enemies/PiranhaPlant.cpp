#include "SuperMarioGame.hpp"
#include "PiranhaPlant.hpp"

PiranhaPlant::PiranhaPlant() {
    auto texture = MARIO_GAME.textureManager().get("Enemies");
    m_animator.create("open", *texture, Vector(32, 80), SIZE, 1, 1, 1);
    m_animator.create("close", *texture, Vector(0, 80), SIZE, 1, 1, 1);
    m_animator.create("biting", *texture, Vector(0, 80), SIZE, 2, 1, 0.01);
}

void PiranhaPlant::takeDamage(DamageType damageType, Character* attacker) {
    if (damageType == DamageType::HIT_FROM_ABOVE) {
        if (m_dead_zone) {
            attacker->takeDamage(DamageType::KICK, this);
        }
    }
    else {
        removeLater();
        addScoreToPlayer(800);
        MARIO_GAME.playSound("kick");
    }
}

bool PiranhaPlant::isAlive() const {
    return true;
}

void PiranhaPlant::touch(Character* character) {
    if (m_dead_zone) {
        character->takeDamage(DamageType::KICK, character);
    }
}

void PiranhaPlant::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

void PiranhaPlant::update(int delta_time) {
    Enemy::update(delta_time);
    m_animator.update(delta_time);

    m_timer += delta_time;

    m_dead_zone = ((m_timer > 0.25 * PERIOD_MS) &&
        (m_timer < 3 * PERIOD_MS));

    float height = 0;

    if (m_timer < PERIOD_MS) { // none
        height = 0;
        // too close to mario - don't appear
        if ((mario()->getBounds().center() - getBounds().center()).length() < 100) {
            hideInTube();
        }
    }
    else if (m_timer < 1.25 * PERIOD_MS) { // appearing
        playAnimation("open");
        height = ((m_timer - PERIOD_MS) / (0.25f * PERIOD_MS)) * SIZE.y;
    }
    else if (m_timer < 3 * PERIOD_MS) {    // in full size
        playAnimation("biting");
        height = SIZE.y;
    }
    else if (m_timer < 3.25 * PERIOD_MS) { // hiding
        playAnimation("close");
        height = (1 - ((m_timer - 3 * PERIOD_MS) / (0.25f * PERIOD_MS))) * SIZE.y;
    }
    else {
        m_timer = 0;
    }

    setSize({ SIZE.x, height });
    move({ 0.f, m_buttom - getPosition().y - height });
}

void PiranhaPlant::hideInTube() {
    m_timer = 0;
    setSize({ SIZE.x, 0.f });
    move({ 0.f, m_buttom - getPosition().y });
}

void PiranhaPlant::onStarted() {
    Enemy::onStarted();
    m_buttom = getPosition().y + SIZE.y;
    //moveToBack();
}
