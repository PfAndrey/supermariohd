#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "Character.hpp"

class Mario;
class Blocks;

class Enemy : public Character {
    DECLARE_TYPE_INFO(Character)
public:
    static bool isCharacterInFront(Character* target, GameObject* origin);

protected:
 
    enum LogicFlags {
        EMPTY      = 0x0,
        ON_X_BOUND = 0x1,
    };

    void update(int delta_time);
    void onStarted() override;
    Mario* mario();
    void addScoreToPlayer(int score);
    void checkNextTileUnderFoots();
    void checkCollideOtherCharasters();
    void checkFallUndergound();
    void updateCollision(float delta_time, LogicFlags logicFlags = LogicFlags::EMPTY);
    void updatePhysics(float delta_time, float gravity);
    bool isInCamera() const;
    void playAnimation(const std::string& name);
    void playSound(const std::string& name);
    void setVelocity(const Vector& velocity) { m_velocity = velocity; }

    static constexpr float GRAVITY_FORCE = 0.0015f;
    static constexpr float RUN_SPEED = -0.05f;
    static constexpr float SHELL_SLIDING_SPEED = 6 * RUN_SPEED;

    Vector m_velocity;
    Vector m_direction = Vector::LEFT;
    Blocks* m_blocks = nullptr;
    Animator m_animator;
    ECollisionTag m_collision_tag;

private:
    Mario* m_mario = nullptr;
};

#endif // ENEMY_HPP
