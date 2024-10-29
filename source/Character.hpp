#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include "GameEngine.hpp"

enum class DamageType : uint8_t {
    KICK           = 0,
    SHOOT          = 1,
    HIT_FROM_ABOVE = 2,
    HIT_FROM_BELOW = 3
};

class Character : public GameObject {
    DECLARE_TYPE_INFO(GameObject)

public:
    virtual bool isAlive() const = 0;
    virtual void takeDamage(DamageType damageType, Character* attacker) = 0;
    virtual void touch(Character* character) = 0;
};

#endif // CHARACTER_HPP
