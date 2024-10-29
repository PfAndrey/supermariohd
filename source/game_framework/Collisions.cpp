//!
//! \file       Collisions.hpp
//! \author     Andriy Parfenyuk
//! \date       14.3.2017
//!
//! \brief      Collisions detection and response.
//!
//! \license    GNU
//!
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------

#include "Collisions.hpp"

//---------------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

ECollisionTag operator |= (ECollisionTag& one, const ECollisionTag& two)
{
    one = ECollisionTag(static_cast<int>(one) | static_cast<int>(two));
    return one;
}
//---------------------------------------------------------------------------
bool operator & (ECollisionTag one, ECollisionTag  two)
{
    return static_cast<int>(one) & static_cast<int>(two);
}
//---------------------------------------------------------------------------
Vector collisionResponse(const Rect& own_rect, const Vector& own_speed,
                         const Rect& other_rect, const Vector& other_speed,
                         const float delta_time, ECollisionTag& collision_tag)
{
    Rect intersection = other_rect.getIntersection(own_rect);

    if (!intersection.width() || !intersection.height())
    {
        // No collision detected
        return own_rect.leftTop();
    }

    enum Axis
    {
        NONE        = 0,
        VERTICAL    = 1,
        HORIZONTAL  = 2
    };

    Axis axis = NONE;

    float dt = 0;
    Vector delta_speed = own_speed - other_speed;

    if (delta_speed.x && delta_speed.y)
    {
        float dx_t = intersection.width() / std::abs(delta_speed.x);
        float dy_t = intersection.height() / std::abs(delta_speed.y);
        dt = std::min(dx_t, dy_t);
        axis = (dx_t > dy_t) ? Axis::VERTICAL
                             : Axis::HORIZONTAL;
    }
    else if (delta_speed.x)
    {
        dt = intersection.width() / std::abs(delta_speed.x);
        axis = Axis::HORIZONTAL;
    }
    else if (delta_speed.y)
    {
        dt = intersection.height() / std::abs(delta_speed.y);
        axis = Axis::VERTICAL;
    }

    if (dt > delta_time)
    {
        dt = delta_time;
    }

    Vector new_pos = own_rect.leftTop() - dt * delta_speed;

    if (axis == Axis::VERTICAL)
    {
        if (intersection.top() == other_rect.top())
        {
            collision_tag |= ECollisionTag::FLOOR;
        }
        else if (intersection.bottom() == other_rect.bottom())
        {
            collision_tag |= ECollisionTag::CELL;
        }
    }
    else if (axis == Axis::HORIZONTAL)
    {
        if (intersection.left() == other_rect.left())
        {
            collision_tag |= ECollisionTag::RIGHT;
        }
        else if (intersection.right() == other_rect.right())
        {
            collision_tag |= ECollisionTag::LEFT;
        }
    }

    return new_pos;
}

//---------------------------------------------------------------------------
// End of File
//---------------------------------------------------------------------------
