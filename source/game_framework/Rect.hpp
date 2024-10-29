#ifndef RECT_HPP
#define RECT_HPP

#include "Vector.hpp"


class Rect {
public:
    Rect() = default;
    Rect(float left, float top, float width, float height);
    Rect(const Vector& top_left, const Vector& size);
    float top() const;
    float left() const;
    float right() const;
    float bottom() const;
    Vector leftTop() const;
    Vector rightTop() const;
    Vector leftBottom() const;
    Vector rightBottom() const;
    Vector center() const;
    Vector size() const;
    bool isContain(const Vector& point) const;
    bool isContain(const Rect& rect) const;
    bool isContainByX(const Vector& point) const;
    bool isContainByY(const Vector& point) const;
    bool isIntersect(const Rect& other) const;
    Rect getIntersection(const Rect& other) const;
    Rect bordered(float k) const;
    Rect operator* (float k) const;
    Rect operator/ (float k) const;
    bool operator==(const Rect& other) const;
    float width() const;
    float height() const;
    void setWidth(int width);
    void setHeight(int height);
    void setTop(int top);
    void setBottom(int bottom);
    void setRight(int right);
    void setLeft(int left);
    void normalize();
    void move(const Vector& diff);
    void scale(float k);
    Rect moved(const Vector& diff) const;
    Rect scaled(float k) const;

private:
    float m_left = 0.f;
    float m_top = 0.f;
    float m_width = 0.f;
    float m_height = 0.f;
};

Vector operator* (const float& k, const Vector& vector);

#endif // !RECT_HPP
