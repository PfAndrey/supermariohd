#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <unordered_map>
#include "SFML/Graphics.hpp"

class Vector {
public:
    Vector() = default;
    Vector(float x, float y);
    Vector operator-() const;
    Vector operator+ (const Vector& two) const;
    Vector operator- (const Vector& two) const;
    Vector operator* (const float& k) const;
    Vector operator/ (const float& k) const;
    void operator+= (const Vector& two);
    void operator-= (const Vector& two);
    bool operator == (const Vector& other) const;
    bool operator != (const Vector& other) const;
    void operator*= (const float& k);
    float length() const;
    float angle() const;
    Vector normalized() const;
    void normalize();
    static Vector moveTowards(const Vector& current, const Vector& target, float distance);
    bool operator < (const Vector& other) const;
    const static Vector RIGHT;
    const static Vector LEFT;
    const static Vector UP;
    const static Vector DOWN;
    const static Vector ZERO;

    static Vector fromString(const std::string& str);

    Vector(const sf::Vector2f& vector);
    operator sf::Vector2f() const;

    float x = 0;
    float y = 0;
};

Vector round(const Vector& vector);
Vector ceil(const Vector& vector);

std::ostream& operator << (std::ostream& str, const Vector& vector);

#endif // !VECTOR_HPP
