#pragma once

#include "Enemy.hpp"

class PiranhaPlant : public Enemy {
    DECLARE_TYPE_INFO(Enemy)

public:
    PiranhaPlant();
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    void onStarted() override;
    void hideInTube();
    //! ICharacter impl:
    void takeDamage(DamageType damageType, Character* attacker) override;
    void touch(Character* character) override;
    bool isAlive() const override;

private:
    static constexpr float PERIOD_MS = 2000;
    const Vector SIZE = { 32, 46 };
    float m_timer = 0;
    float m_buttom = 0; // <! Initial buttom line
    bool m_dead_zone = false;
};
