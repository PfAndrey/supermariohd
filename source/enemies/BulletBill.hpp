#pragma once

#include "Enemy.hpp"

class BulletBill : public Enemy {
public:
    BulletBill(const Vector& infitial_pos, const Vector& initial_speed);
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    //! ICharacter impl:
    void takeDamage(DamageType damageType, Character* attacker) override;
    void touch(Character* character) override;
    bool isAlive() const override;

private:

    enum class State : uint8_t {
        NORMAL = 0,
        DIED   = 1
    };

    void setState(State state);
    State m_state = State::NORMAL;
};

class BulletBillSpawner : public GameObject {
public:
    BulletBillSpawner();
    void update(int delta_time) override;
    void onStarted() override;

private:
    bool isBulletBillBeyondTiledMap() const;

    Mario* m_mario = nullptr;
    int m_blocks_width = 0;
    static constexpr int SPAWN_INTERVAL = 4000;
    static constexpr float BULLET_SPEED = 0.15f;
    float m_spawn_timer = 0;
};
