#pragma once

#include "Enemy.hpp"

class Spinny : public Enemy {
public:
    Spinny(const Vector& position, const Vector& speed, const Vector& walk_direction);
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    void takeDamage(DamageType damageType, Character* attacker) override;
    void touch(Character* character) override;
    bool isAlive() const override;

private:

    enum State : uint8_t {
        EGG = 0,
        NORMAL = 1,
        DIED = 2
    };

    void setState(State state);

    State m_state = State::EGG;
    Vector m_walk_direction;
};

class Lakity : public Enemy {
    DECLARE_TYPE_INFO(Enemy)

public:
    Lakity();
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    void runAway(const Vector& run_direction);
    //! ICharacter impl:
    void takeDamage(DamageType damageType, Character* attacker) override;
    void touch(Character* character) override;
    bool isAlive() const override;

private:
    enum class State : uint8_t {
        NORMAL = 0,
        DIED = 1,
        RUN_AWAY = 2
    };

    void setState(State state);

    static constexpr int FIRE_RATE = 2500;
    State m_state = State::NORMAL;
    float m_fire_timer = 0;
    float m_died_timer = 0;
};

class LakitySpawner : public GameObject {
public:
    LakitySpawner() = default;
    void update(int delta_time) override;

private:
    void onStarted() override;

    static constexpr int CHECK_INTERVAL = 5000;
    Lakity* m_lakity = nullptr;
    Mario* m_mario = nullptr;
    float m_lakity_checker_timer = 0;
};
