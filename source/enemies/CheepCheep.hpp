#pragma once

#include "Enemy.hpp"

class CheepCheep : public Enemy {
public:
    CheepCheep();
    CheepCheep(const Vector& initial_pos, const Vector& initial_speed);
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    //! ICharacter impl:
    void takeDamage(DamageType damageType, Character* attacker) override;
    void touch(Character* character) override;
    bool isAlive() const override;

private:

    enum class State : uint8_t {
        NORMAL = 0,
        UNDERWATER = 1,
        DIED = 2
    };

    void setState(State state);
    State m_state = State::NORMAL;
};

class CheepCheepSpawner : public GameObject {
public:
    CheepCheepSpawner() = default;
    void update(int delta_time) override;

private:
    void onStarted() override;

    static constexpr int SPAWN_INTERVAL = 1000;
    const Vector spawn_speed = { 0.32f,-0.65f };
    float m_spawn_timer = 0;
    int m_map_height;
    Mario* m_mario = nullptr;
};
