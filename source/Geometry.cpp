
#include "Geometry.h"
#include <math.h>

Vector rotateClockwise(const Vector& direction)
{
    return Vector(-direction.y, direction.x);
}

Vector rotateAnticlockwise(const Vector& direction)
{
    return Vector(direction.y, -direction.x);
}

int getLength(const std::vector<Vector>& path)
{
    int length = 0;
    for (int i = 1; i < path.size(); ++i)
        length += (path[i] - path[i - 1]).length();
    return length;
}

Vector toVector(const std::string& str)
{
    if (str == "Left" || str == "left") return Vector::left;
    if (str == "Right" || str == "right") return Vector::right;
    if (str == "Down" || str == "down") return Vector::down;
    if (str == "Up" || str == "up") return Vector::up;
    return Vector::zero;
}

//-----------------------------------------------------------------------------

Vector::Vector(float x, float y) : x(x), y(y) {}
Vector::Vector()
{
    x = y = 0;
}

Vector Vector::operator + (const Vector& two) const
{
    return Vector(x + two.x, y + two.y);
}

Vector Vector::operator- (const Vector& two) const
{
    return Vector(x - two.x, y - two.y);
}

Vector Vector::operator* (const float& k) const
{
    return Vector(x*k, y*k);
}

void Vector::operator*= (const float& k)
{
    x *= k;
    y *= k;
}

void Vector::operator+= (const Vector& two)
{
    x += two.x;
    y += two.y;
}

void Vector::operator-= (const Vector& two)
{
    x -= two.x;
    y -= two.y;
}

Vector Vector::operator/ (const float& k) const
{
    return Vector(x / k, y / k);
}

Vector Vector::operator-() const
{
    return Vector(-x, -y);
}

bool Vector::operator == (const Vector& other) const
{
    return (other.x == x && other.y == y);
}

bool Vector::operator != (const Vector& other) const
{
    return (other.x != x || other.y != y);
}

float Vector::length() const
{
    return sqrt(x*x + y*y);
}

float Vector::angle() const
{
    float angle = atan2(x, y) / 3.1415 * 180;
    if (angle < 0)
        angle += 360;
    return angle;
}

Vector Vector::normalized() const
{
    auto l = length();
    return Vector(x / l, y / l);
}

void Vector::normalize()
{
	auto l = length();
	x /= l;
	y /= l;
}

Vector::Vector(const sf::Vector2f& vector)
{
    x = vector.x;
    y = vector.y;
}

Vector::operator sf::Vector2f() const
{
    return sf::Vector2f(x, y);
}

Vector round(const Vector& vector)
{
    return Vector(std::roundf(vector.x), std::roundf(vector.y));
}

Vector floor(const Vector& vector)
{
    return Vector(int(vector.x), int(vector.y));
}

Vector operator* (const float& k, const Vector& vector)
{
    return Vector(vector.x*k, vector.y*k);
}

Vector Vector::moveTowards(const Vector& current, const Vector& target, float distance)
{
    return (target - current).normalized()*distance + current;
}

bool Vector::operator < (const Vector& other) const
{
    return y * 10000 + x < other.y * 10000 + other.x;
}

const Vector Vector::right = {1,0};
const Vector Vector::left = {-1,0};
const Vector Vector::up = {0,-1};
const Vector Vector::down = {0,1};
const Vector Vector::zero = {0,0};

//----------------------------------------------------------------------------------

Rect::Rect()
{
    _left = _top = _width = _height = 0;
}

Rect::Rect(float left, float top, float width, float height) : _left(left), _top(top), _width(width), _height(height)
{

}

Rect::Rect(const Vector& top_left, const Vector& size):
    _left(top_left.x), _top(top_left.y),
    _width(size.x), _height(size.y)
{

}

float Rect::right() const
{
    return _left + _width;
}

float Rect::bottom() const
{
    return _top + _height;
}

Vector Rect::leftTop() const
{
    return {_left,_top};
}

Vector Rect::rightTop() const
{
    return {right(),_top};
}

Vector Rect::leftBottom() const
{
    return {_left,bottom()};
}

Vector Rect::rightBottom() const
{
    return {right(),bottom()};
}

Vector Rect::center() const
{
    return {_left + _width / 2, _top + _height / 2};
}

Vector Rect::size() const
{
    return {_width,_height};
}

std::ostream& operator << (std::ostream& str, const Vector& vector)
{
    str << "[" << vector.x << "," << vector.y << "]";
    return str;
}

bool Rect::isContain(const Vector& point) const
{
    if (point.x >= _left && point.y >= _top &&
            point.x < right() && point.y < bottom())
        return true;
    return false;
}

bool Rect::isContain(const Rect& other_rect) const
{
    if (other_rect._left >= _left && other_rect.right() <= right() &&
            other_rect._top >= _top && other_rect.bottom() <= bottom())
        return true;
    return false;
}

bool Rect::isContainByX(const Vector& point) const
{
    if (point.x >= _left  && point.x < right())
        return true;
    return false;
}

bool Rect::isContainByY(const Vector& point) const
{
    if (point.y >= _top && point.y < bottom())
        return true;
    return false;
}

bool Rect::isIntersect(const Rect& other) const
{	
    return     abs(2*(_left - other._left) + _width  - other._width ) < abs(_width  + other._width )
            && abs(2*(_top - other._top) + _height   - other._height ) < abs(_height + other._height );
}

Rect Rect::getIntersection(const Rect& other) const
{
    Rect new_rect;
    new_rect._left = std::max(_left, other._left);
    new_rect._top = std::max(_top, other._top);
    new_rect._height = std::min(bottom(), other.bottom()) - new_rect._top;
    new_rect._width = std::min(right(), other.right()) - new_rect._left;
    return new_rect;
}

Rect Rect::bordered(float k) const
{
    return{ _left - k, _top - k, _width + 2 * k, _height + 2 * k };
}

Rect Rect::operator* (float k) const
{
    return{ _left*k,_top*k,_width*k,_height*k };
}

float Rect::top() const
{
    return _top;
}

float Rect::left() const
{
    return _left;
}

bool Rect::operator==(const Rect& other) const
{
    if (_left == other._left && _top == other._top &&
            _width == other._width && _height == other._height)
        return true;
    return false;
}

float Rect::width() const
{
    return _width;
}

float Rect::height() const
{
    return _height;
}

void Rect::setWidth(int width)
{
    _width = width;
}

void Rect::setHeight(int height)
{
    _height = height;
}

void Rect::setTop(int top)
{
    _height += _top - top;
    _top = top;
}

void Rect::setBottom(int bottom)
{
    _height = bottom - _top;
}

void Rect::setRight(int right)
{
    _width =   right - _left;
}

void Rect::setLeft(int left)
{
    _width += _left - left;
    _left = left;
}

void Rect::normalize()
{
    if (_width < 0)
    {
        _left += _width;
        _width = -_width;
    }
    if (_height < 0)
    {
        _top += _height;
        _height = -_height;
    }
}

Rect Rect::moved(const Vector& diff) const
{
    return Rect(_left + diff.x, _top + diff.x, _width, _height);
}
