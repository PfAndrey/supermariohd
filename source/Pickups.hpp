
#ifndef PICKUPS_HPP
#define PICKUPS_HPP

#include "GameEngine.hpp"

class Mario;
class Blocks;

class Coin : public GameObject {
    DECLARE_TYPE_INFO(GameObject)

public:
    Coin();
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    void kick();

private:

    enum class State {
        TWIST = 0,
        SHANE = 1
    };
 
    void onStarted() override;
    State m_state = State::TWIST;
    int m_remove_timer = 1000;
    Animator m_animator;
    Mario* m_mario = nullptr;
};

class Mushroom : public GameObject {
    DECLARE_TYPE_INFO(GameObject)

public:
    Mushroom(const Vector& pos);
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    void kick();

protected:
    void onStarted() override;
    virtual void action();
    sf::Sprite m_sprite;
    bool m_as_flower = false;

private:
    enum class State {
        WAIT    = 0,
        GROVING = 1,
        NORMAL  = 2
    };

    const float GRAVITY_FORCE = 0.0015f;
    const float GROVING_RATE = 0.02f;
    const float RUN_SPEED = 0.1f;
    State m_state = State::WAIT;
    Vector m_speed;
    float m_timer = 0;
    Mario* m_mario = nullptr;
    Blocks* m_blocks = nullptr;
};

class FireFlower : public GameObject {
    DECLARE_TYPE_INFO(GameObject)

public:
    FireFlower(const Vector& pos);
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;

protected:

    void onStarted() override;
    virtual void action();
    sf::Sprite m_sprite;

private:
    enum class State {
        WAIT = 0,
        GROVING = 1,
        NORMAL = 2
    };

    const float GRAVITY_FORCE = 0.0015f;
    const float GROVING_RATE = 0.02f;
    State m_state = State::WAIT;
    Vector m_speed;
    float m_timer = 0;
    Mario* m_mario = nullptr;
    Blocks* m_blocks = nullptr;
};

class OneUpMushroom : public Mushroom {
public:
    OneUpMushroom(const Vector& pos);

private:
    void action() override;
};

class Star : public GameObject {
public:
    Star(const Vector& pos);
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;

private:
    enum State {
        WAIT    = 1,
        BORNING = 2,
        NORMAL  = 3
    };

    void onStarted() override;

    const float GRAVITY_FORCE = 0.0015f;
    const float JUMP_POWER = 0.6f;
    const float HORIZONTAL_SPEED = 0.1f;
    State m_state = State::WAIT;
    sf::Sprite m_sprite;
    float m_timer = 0;
    Vector m_speed;
    Mario* m_mario = nullptr;
    Blocks* m_blocks = nullptr;
};

#endif // !PICKUPS_HPP
