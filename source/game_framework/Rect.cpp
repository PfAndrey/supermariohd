#include "Rect.hpp"


Rect::Rect(float left, float top, float width, float height)
    : m_left(left)
    , m_top(top)
    , m_width(width)
    , m_height(height) {
}

Rect::Rect(const Vector& top_left, const Vector& size)
    : Rect(top_left.x, top_left.y, size.x, size.y) {
}

float Rect::right() const {
    return m_left + m_width;
}

float Rect::bottom() const {
    return m_top + m_height;
}

Vector Rect::leftTop() const {
    return {m_left, m_top};
}

Vector Rect::rightTop() const {
    return {right(), m_top};
}

Vector Rect::leftBottom() const {
    return {m_left, bottom()};
}

Vector Rect::rightBottom() const {
    return {right(), bottom()};
}

Vector Rect::center() const {
    return {m_left + m_width / 2, m_top + m_height / 2};
}

Vector Rect::size() const {
    return {m_width, m_height};
}

bool Rect::isContain(const Vector& point) const {
    return ((point.x >= m_left) && (point.y >= m_top) &&
            (point.x < right()) && (point.y < bottom()));
}

bool Rect::isContain(const Rect& other_rect) const {
    return ((other_rect.m_left >= m_left) && (other_rect.right() <= right()) &&
            (other_rect.m_top >= m_top) && (other_rect.bottom() <= bottom()));
}

bool Rect::isContainByX(const Vector& point) const {
    return ((point.x >= m_left) && (point.x < right()));
}

bool Rect::isContainByY(const Vector& point) const {
    return ((point.y >= m_top) && (point.y < bottom()));
}

bool Rect::isIntersect(const Rect& other) const {
    if (!(abs(2 * (m_left - other.m_left) + m_width - other.m_width) < abs(m_width + other.m_width))) {
        return false;
    }

    if (!(abs(2 * (m_top - other.m_top) + m_height - other.m_height) < abs(m_height + other.m_height))) {
        return false;
    }

    return true;
}

Rect Rect::getIntersection(const Rect& other) const {
    Rect new_rect;
    new_rect.m_left = std::max(m_left, other.m_left);
    new_rect.m_top  = std::max(m_top, other.m_top);
    new_rect.m_height = std::min(bottom(), other.bottom()) - new_rect.m_top;
    new_rect.m_width  = std::min(right(), other.right()) - new_rect.m_left;
    return new_rect;
}

Rect Rect::bordered(float k) const {
    return {m_left - k, m_top - k, m_width + 2 * k, m_height + 2 * k};
}

Rect Rect::operator* (float k) const {
    return {m_left * k, m_top * k, m_width * k, m_height * k};
}

Rect Rect::operator/ (float k) const {
    return { m_left / k, m_top / k, m_width / k, m_height / k };
}

float Rect::top() const {
    return m_top;
}

float Rect::left() const {
    return m_left;
}

bool Rect::operator==(const Rect& other) const {
    return ((m_left == other.m_left) && (m_top == other.m_top) &&
            (m_width == other.m_width) && (m_height == other.m_height));
}

float Rect::width() const {
    return m_width;
}

float Rect::height() const {
    return m_height;
}

void Rect::setWidth(int width) {
    m_width = width;
}

void Rect::setHeight(int height) {
    m_height = height;
}

void Rect::setTop(int top) {
    m_height += m_top - top;
    m_top = top;
}

void Rect::setBottom(int bottom) {
    m_height = bottom - m_top;
}

void Rect::setRight(int right) {
    m_width = right - m_left;
}

void Rect::setLeft(int left) {
    m_width += m_left - left;
    m_left = left;
}

void Rect::normalize() {
    if (m_width < 0) {
        m_left += m_width;
        m_width = -m_width;
    }

    if (m_height < 0) {
        m_top += m_height;
        m_height = -m_height;
    }
}

Rect Rect::moved(const Vector& diff) const {
    return Rect(m_left + diff.x, m_top + diff.y, m_width, m_height);
}

void Rect::move(const Vector& diff) {
    m_left += diff.x;
    m_top += diff.y;
}

Rect Rect::scaled(float k) const {
    return Rect(m_left * k, m_top * k, m_width * k, m_height * k);
}

void Rect::scale(float k) {
    m_left *= k;
    m_top *= k;
    m_width *= k;
    m_height *= k;
}
