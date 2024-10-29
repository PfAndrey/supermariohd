#include <cmath>

#include <Format.hpp>
#include "Vector.hpp"

const Vector Vector::RIGHT = { 1, 0  };
const Vector Vector::LEFT  = { -1, 0 };
const Vector Vector::UP    = { 0, -1 };
const Vector Vector::DOWN  = { 0, 1  };
const Vector Vector::ZERO  = { 0, 0  };

Vector::Vector(float x, float y)
    : x(x)
    , y(y)
{
}

Vector Vector::operator + (const Vector& two) const {
    return Vector(x + two.x, y + two.y);
}

Vector Vector::operator- (const Vector& two) const {
    return Vector(x - two.x, y - two.y);
}

Vector Vector::operator* (const float& k) const {
    return Vector(x * k, y * k);
}

void Vector::operator*= (const float& k) {
    x *= k;
    y *= k;
}

void Vector::operator+= (const Vector& two) {
    x += two.x;
    y += two.y;
}

void Vector::operator-= (const Vector& two) {
    x -= two.x;
    y -= two.y;
}

Vector Vector::operator/ (const float& k) const {
    return Vector(x / k, y / k);
}

Vector Vector::operator-() const {
    return Vector(-x, -y);
}

bool Vector::operator == (const Vector& other) const {
    return ((other.x == x) && (other.y == y));
}

bool Vector::operator != (const Vector& other) const {
    return ((other.x != x) || (other.y != y));
}

float Vector::length() const {
    return sqrt(x * x + y * y);
}

float Vector::angle() const {
    float angle = atan2f(x, y) / 3.1415 * 180;

    if (angle < 0) {
        angle += 360;
    }

    return angle;
}

Vector Vector::normalized() const {
    auto l = length();
    return Vector(x / l, y / l);
}

void Vector::normalize() {
    auto l = length();
    x /= l;
    y /= l;
}

Vector::Vector(const sf::Vector2f& vector) {
    x = vector.x;
    y = vector.y;
}

Vector::operator sf::Vector2f() const {
    return sf::Vector2f(x, y);
}

Vector floor(const Vector& vector) {
    return Vector(int(vector.x), int(vector.y));
}

Vector operator* (const float& k, const Vector& vector) {
    return Vector(vector.x*k, vector.y*k);
}

Vector Vector::moveTowards(const Vector& current, const Vector& target, float distance) {
    return (target - current).normalized() * distance + current;
}

bool Vector::operator < (const Vector& other) const {
    return y * 10000 + x < other.y * 10000 + other.x;
}

std::ostream& operator << (std::ostream& str, const Vector& vector) {
    str << "[" << vector.x << "," << vector.y << "]";
    return str;
}

Vector round(const Vector& vector) {
    return Vector(std::round(vector.x), std::round(vector.y));
}

Vector ceil(const Vector& vector) {
    return Vector(std::ceil(vector.x), std::ceil(vector.y));
}

Vector Vector::fromString(const std::string& str) {
    using namespace utils;

    switch (strHash(str.c_str())) {
    case strHash("Right"): // fall through
    case strHash("right"):
        return Vector::RIGHT;
    case strHash("Left"): // fall through
    case strHash("left"):
        return Vector::LEFT;
    case strHash("Down"): // fall through
    case strHash("down"):
        return Vector::DOWN;
    case strHash("Up"): // fall through
    case strHash("up"):
        return Vector::UP;
    default:
        return Vector::ZERO;
    }
}
